# FTE Hybrid Server Entity Baseline Issue

## Summary

When connecting to FTE hybrid (NetQuake/QuakeWorld) servers, entities (weapons, health, armor, lifts) are invisible on first connect but work correctly after a manual `reconnect`. This document describes the root cause and current workaround.

## Background: Quake Network Protocols

### NetQuake vs QuakeWorld

**NetQuake (NQ)** was the original Quake network protocol (1996). It was designed for LAN play and assumed reliable, low-latency connections. Key characteristics:
- Client sends input, waits for server response before rendering
- No client-side prediction
- High latency feels sluggish and unplayable
- Simple protocol, server is authoritative

**QuakeWorld (QW)** was id Software's response to internet play (1997). It introduced:
- **Client-side prediction**: Client predicts movement locally, server corrects
- **Delta compression**: Only send changes from previous state, not full state
- **Unreliable packets**: Accept packet loss, interpolate missing data
- Different spawn sequence and entity management
- Much better internet play experience

### The Hybrid Server Problem

FTE (ForeThought Engine) is a modern Quake engine that can run as a **hybrid server**, accepting both NetQuake and QuakeWorld clients simultaneously. This is useful for:
- Running modern mods that support both client types
- Allowing players with different clients to play together
- Backwards compatibility

However, hybrid mode creates protocol ambiguity. The server must:
- Detect which protocol the client speaks
- Translate between NQ and QW message formats
- Handle different spawn sequences
- Manage entity state for clients with different expectations

ezQuake is a QuakeWorld client connecting to a hybrid server that's running in a mode that blends both protocols.

### Entity Baselines Explained

In Quake, the game world contains **entities**: players, weapons, health packs, monsters, doors, lifts, torches, etc. Each entity has state:
- Position (origin)
- Rotation (angles)
- Model (what it looks like)
- Animation frame
- Skin, effects, etc.

**Baselines** are the "default" or "initial" state of each entity, sent once when the client connects. They serve as a reference point for delta compression.

#### How Delta Compression Works

Instead of sending full entity state every frame (wasteful), the server sends **deltas** - only what changed from the baseline:

```
Baseline: entity 42 = { model: "progs/armor.mdl", origin: (100, 200, 0), ... }
Frame 1:  entity 42 = { origin: (100, 200, 0) }  // No delta needed, matches baseline
Frame 2:  entity 42 = { origin: (105, 200, 0) }  // Delta: origin changed
```

The delta message includes **bits** (flags) indicating which fields changed:
- `U_ORIGIN1` (0x0002): X position changed
- `U_MODEL` (0x0004): Model changed
- `U_FRAME` (0x0040): Animation frame changed
- etc.

If `bits = 0x0`, nothing changed from baseline - use baseline values.

#### The Baseline Problem

If baselines are never sent:
- All entities have `baseline.modelindex = 0` (no model)
- Server sends `bits = 0x0` (no changes)
- Client applies delta to baseline: `0 + nothing = 0`
- Entity has no model = **invisible**

This is exactly what happens on first connect to the FTE hybrid server.

### Connection Sequence

#### Standard QuakeWorld Connection

1. Client sends connection request with protocol info
2. Server responds with `S2C_CONNECTION`
3. Client sends `"new"` command
4. Server sends `svc_serverdata` (map name, player slot, etc.)
5. Client requests model/sound lists
6. Server sends `svc_spawnbaseline` for each entity ← **Critical step**
7. Server sends `svc_spawnstatic` for static entities (torches, etc.)
8. Server sends `"cmd spawn"` stufftext
9. Client sends `"spawn"` command
10. Client goes active, entities are visible

#### What FTE Hybrid Does (First Connect)

1. Client sends connection request
2. Server responds with `S2C_CONNECTION`
3. Client sends `"new"` command
4. Server sends `svc_serverdata`
5. **Server immediately starts sending `svc_packetentities`** ← Too early!
6. Model/sound lists are sent
7. **`svc_spawnbaseline` is never sent** ← Problem!
8. **`"cmd spawn"` is never sent** ← Problem!
9. Client times out waiting, goes active anyway
10. All entities have `modelindex = 0` = invisible

#### What FTE Hybrid Does (Reconnect)

1. Client sends `"new"` command (via reconnect)
2. Server restarts spawn sequence properly
3. Server sends 65x `svc_fte_spawnbaseline2` ← **Now it works!**
4. Server sends `"cmd spawn"` stufftext
5. Server sends entities with full state (`bits = 0xffff800c` includes U_MODEL)
6. Entities are visible

### SVC Message Types

Key server-to-client (svc) messages mentioned:

| Message | Code | Purpose |
|---------|------|---------|
| `svc_spawnbaseline` | 22 | Set initial state for an entity (standard QW) |
| `svc_fte_spawnbaseline2` | 66 | FTE extended baseline (supports more entities) |
| `svc_spawnstatic` | 20 | Create a static entity (doesn't move, like torches) |
| `svc_packetentities` | 47 | Delta-compressed entity updates |
| `svc_stufftext` | 9 | Server tells client to execute a command |
| `svc_serverdata` | 11 | Initial server info (map, protocol, etc.) |

### CLC Message Types

Key client-to-server (clc) messages:

| Message | Code | Purpose |
|---------|------|---------|
| `clc_stringcmd` | 4 | Client sends a command string ("new", "spawn", etc.) |
| `clc_move` | 3 | Client movement/input |

## Environment

- **Server**: FTE git-6358-6ee5fd584-dirty (hybrid NetQuake/QuakeWorld mode)
- **FTE Protocol Extensions**: 0x21487008 (includes FTE_PEXT_SPAWNSTATIC2), 0x2
- **Client**: ezQuake

## Symptoms

1. First connect to FTE hybrid server
2. Sounds work ✓
3. Sprites work (shotgun effects visible) ✓
4. Map geometry renders correctly ✓
5. **Entities are invisible** (weapons, health, armor, lifts, torches)
6. Entities can be picked up (collision works, positions correct)
7. Manual `reconnect` command fixes the issue - all entities visible

## Root Cause Analysis

### The Problem

The FTE hybrid server never sends `svc_spawnbaseline` or `svc_fte_spawnbaseline2` messages on first connect. These messages establish the default state (including model index) for each entity.

Without baselines:
- All entities have `baseline.modelindex = 0`
- Server sends delta-compressed entity updates with `bits = 0x0` (no changes from baseline)
- Since baseline has no model, and delta has no U_MODEL flag, entities remain invisible

### Evidence from Logs

**First Connect (broken):**
```
[ENTITY 0] num=41 baseline_model=0 bits=0x0 has_model=0
[ENTITY 1] num=43 baseline_model=0 bits=0x0 has_model=0
...
=== CL_ParsePacketEntities: Going active (baselines=0, serverSpawn=0, packets=50) ===
=== CL_MakeActive: num_statics=0 ===
```

**After Reconnect (working):**
```
*** svc_fte_spawnbaseline2 ***  (65 times!)
*** SERVER SENT 'cmd spawn' ***
[ENTITY 0] num=1 baseline_model=0 bits=0xffff800c has_model=1
[ENTITY 1] num=2 baseline_model=0 bits=0xffff800c has_model=1
...
=== CL_ParsePacketEntities: Going active (baselines=65, serverSpawn=1, packets=1) ===
```

### Key Differences

| Aspect | First Connect | After Reconnect |
|--------|---------------|-----------------|
| `svc_fte_spawnbaseline2` | 0 received | 65 received |
| `cmd spawn` stufftext | Not received | Received |
| Entity bits | `0x0` (no data) | `0xffff800c` (full data including U_MODEL) |
| Baselines received | 0 | 65 |
| num_statics | 0 | N/A (not logged) |

### Why Reconnect Works

When the `reconnect` command is executed while connected:

```c
// From cl_main.c CL_Reconnect_f()
if (cls.state == ca_connected) {
    Com_Printf ("reconnecting...\n");
    MSG_WriteChar (&cls.netchan.message, clc_stringcmd);
    MSG_WriteString (&cls.netchan.message, "new");
    return;
}
```

The `"new"` command tells the FTE server to restart the spawn sequence, which includes:
1. Sending `svc_fte_spawnbaseline2` for all entities
2. Sending `svc_spawnstatic` for static entities
3. Sending `cmd spawn` stufftext
4. Sending entity updates with full state (bits include U_MODEL)

### Why First Connect Fails

On first connect, ezQuake sends `"new"` immediately after receiving `S2C_CONNECTION`:

```c
// From cl_main.c CL_ConnectionlessPacket()
case S2C_CONNECTION:
    Netchan_Setup(NS_CLIENT, &cls.netchan, net_from, cls.qport, 0);
    MSG_WriteChar (&cls.netchan.message, clc_stringcmd);
    MSG_WriteString (&cls.netchan.message, "new");
    cls.state = ca_connected;
```

However, the FTE hybrid server appears to:
1. Start streaming `svc_packetentities` immediately
2. Process our `"new"` command too late (or differently for new vs existing clients)
3. Never send baselines because it thinks we're an existing client

## Current Workaround

The current fix automatically triggers a `reconnect` when we detect the baseline issue:

```c
// In CL_ParsePacketEntities() - cl_ents.c
if (timeout && !hasBaselines && !serverReady && !cl.sentNewRequest) {
    Com_Printf("=== FTE fix: No baselines received, scheduling reconnect ===\n");
    cl.sentNewRequest = true; // Prevent repeated reconnects
    Cbuf_AddText("reconnect\n");
    return; // Don't go active, wait for reconnect
}
```

This causes a brief reconnection but results in fully functional entity rendering.

## Tracking Variables Added

In `clientState_t` (client.h):
- `int baselinesReceived` - Count of baselines received from server
- `int prespawnPacketCount` - Packets received since prespawn (for timeout)
- `qbool serverSentSpawnCmd` - True if server sent "cmd spawn" stufftext
- `qbool sentNewRequest` - True if we've already tried the reconnect fix

## Future Investigation

To properly fix this without the reconnect workaround, investigate:

1. **FTE Server Behavior**: Why does FTE send baselines on reconnect but not first connect?
   - Is there a different client flag/state we should signal?
   - Does FTE expect a specific command sequence?

2. **Protocol Extension Flags**: Are there FTE extensions that control baseline behavior?
   - Check FTE_PEXT flags for baseline-related options
   - May need to request specific behavior via protocol negotiation

3. **Timing**: The `"new"` command we send at S2C_CONNECTION might be:
   - Arriving too late (server already streaming)
   - Being processed differently for "new connection" vs "reconnection"

4. **Server-Side State**: FTE may track per-client baseline state
   - First connect: assumes client has no state, doesn't send baselines
   - Reconnect: knows client had state, sends fresh baselines

5. **Alternative Commands**: Check if there's an FTE-specific command to request baselines
   - Commands like "prespawn", "spawn", "begin" may have different effects
   - FTE may have custom commands for hybrid mode

## Files Modified

- `src/client.h` - Added tracking variables to clientState_t
- `src/cl_ents.c` - Added baseline detection and auto-reconnect workaround
- `src/cl_parse.c` - Added baseline counting and spawn command detection

## Related Issues Fixed in This Session

1. ✅ Connection crashes - Fixed
2. ✅ Audio deadlock - Fixed  
3. ✅ Collision race conditions - Fixed
4. ✅ Frame mismatch with delta compression - Fixed
5. ✅ Sounds not working on first connect - Fixed
6. ✅ Sprites not working on first connect - Fixed
7. ⚠️ Entities invisible on first connect - **Workaround applied** (auto-reconnect)

## Date

January 9, 2026
