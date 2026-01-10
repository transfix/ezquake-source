# ezQuake Architecture & Networking Technical Report
## A Foundation for Quake RTS: Massive Real-Time Strategy

This report provides a comprehensive technical analysis of the ezQuake codebase, with emphasis on networking, QuakeC integration, and game architecture. **This document also serves as a planning guide for an ambitious vision: a Quake-based massive real-time strategy game where individual players can take the role of units in battles, with federated multi-server architecture supporting massive numbers of bots, structures, and dynamic content.**

---

## Table of Contents

1. [Server Code & Build Modes](#1-server-code--build-modes)
2. [NetQuake Protocol Support](#2-netquake-protocol-support)
3. [Network Architecture](#3-network-architecture)
4. [Client-Side Prediction](#4-client-side-prediction)
5. [Player Limits](#5-player-limits)
6. [Bot Implementation](#6-bot-implementation)
7. [Map & Entity Limits](#7-map--entity-limits)
8. [Asset Downloading](#8-asset-downloading)
9. [Time Quantization & Rendering Loops](#9-time-quantization--rendering-loops)
10. [Physics System](#10-physics-system)
11. [Shared Client/Server Code](#11-shared-clientserver-code)
12. [Console System](#12-console-system)
13. [QuakeC Virtual Machine](#13-quakec-virtual-machine)
14. [Extending QuakeC](#14-extending-quakec)
15. [Dynamic Entity Management](#15-dynamic-entity-management)
16. [Embedding Scripting Interpreters](#16-embedding-scripting-interpreters)
17. [Threading Model](#17-threading-model)
18. [Scaling Entities to 65536 and Beyond](#18-scaling-entities-to-65536-and-beyond)
19. [Model Technical Limits](#19-model-technical-limits)
20. [Bandwidth Analysis & Scaling](#20-bandwidth-analysis--scaling)
21. [Federated Multi-Server Architecture](#21-federated-multi-server-architecture)
22. [Dynamic Media Loading](#22-dynamic-media-loading)
23. [Vision: Quake Massive RTS](#23-vision-quake-massive-rts)
24. [Rendering Pipeline & Performance](#24-rendering-pipeline--performance)
25. [Dynamic QuakeC & Hot-Reload](#25-dynamic-quakec--hot-reload)
26. [Volume Rendering Integration](#26-volume-rendering-integration)
27. [Menu System Architecture](#27-menu-system-architecture)
28. [Text Rendering System](#28-text-rendering-system)

---

## 1. Server Code & Build Modes

### Does ezQuake Include Server Code?

**Yes.** ezQuake includes a full QuakeWorld server implementation. The codebase supports three build configurations controlled by preprocessor macros:

| Macro | Description |
|-------|-------------|
| `SERVERONLY` | Dedicated server only (no rendering/client) |
| `CLIENTONLY` | Client only (no local server) |
| Neither | Combined client+server (default "listen server" mode) |

### Key Server Files

| File | Purpose |
|------|---------|
| [src/sv_main.c](src/sv_main.c) | Main server logic, frame processing, client management |
| [src/sv_phys.c](src/sv_phys.c) | Server-side physics simulation |
| [src/sv_user.c](src/sv_user.c) | Client command processing, downloads |
| [src/sv_send.c](src/sv_send.c) | Network message transmission |
| [src/sv_ents.c](src/sv_ents.c) | Entity state management and delta encoding |
| [src/sv_init.c](src/sv_init.c) | Map loading, server initialization |

### Running a Full Server

When built without `CLIENTONLY`, you can run a listen server by:
1. Starting the client normally
2. Using `map <mapname>` to start a local server
3. Other players can connect to your IP

For dedicated servers, build with `SERVERONLY` defined, or use a separate MVDSV server.

---

## 2. NetQuake Protocol Support

### Hybrid QuakeWorld/NetQuake Support

ezQuake includes **conditional NetQuake progs support** via the `WITH_NQPROGS` compile flag, defined in [CMakeLists.txt#L836](CMakeLists.txt#L836).

```c
// From src/pr_edict.c
#define NQ_PROGHEADER_CRC 5927

#ifdef WITH_NQPROGS
qbool pr_nqprogs;
```

### How It Works

- The server detects NQ progs by CRC checking the loaded `progs.dat`
- When `pr_nqprogs` is true, field and global offsets are patched to match NQ layout
- Physics and entity handling adapt (e.g., missile spawning in [sv_phys.c#L927](src/sv_phys.c#L927))

### Cvar Control

```c
cvar_t sv_forcenqprogs = {"sv_forcenqprogs", "0"};
```

### Limitations

- This is **server-side only** - ezQuake remains a QuakeWorld client
- The client cannot natively connect to pure NetQuake servers
- Primarily enables running single-player NQ mods on the QW engine

---

## 3. Network Architecture

### Core Networking Files

| File | Purpose |
|------|---------|
| [src/net.c](src/net.c) | Low-level socket operations, UDP handling |
| [src/net_chan.c](src/net_chan.c) | Reliable message channels, packet sequencing |

### Connection Flow

#### Client Initiating Connection

```
┌─────────────────────────────────────────────────────────────────┐
│ CLIENT                          SERVER                         │
├─────────────────────────────────────────────────────────────────┤
│ 1. Send "getchallenge"   ────▶                                  │
│                          ◀────  2. S2C_CHALLENGE + challenge#   │
│                                    + protocol extensions        │
│ 3. Send "connect" + challenge                                   │
│    + protocol version    ────▶                                  │
│    + extensions + userinfo                                      │
│                          ◀────  4. S2C_CONNECTION               │
│ 5. Netchan_Setup()                Netchan_Setup()               │
│    cls.state = ca_connected                                     │
│ 6. Send clc_stringcmd "new" ─▶                                  │
│                          ◀────  7. svc_serverdata               │
│                                    + model/sound precaches      │
│ 8. Send "spawn"          ────▶                                  │
│                          ◀────  9. svc_spawnbaseline            │
│                                    + svc_stufftext              │
│ 10. Send "begin"         ────▶                                  │
│                          ◀──── 11. Full game state              │
│    cls.state = ca_active                                        │
└─────────────────────────────────────────────────────────────────┘
```

#### Key Functions

**Client Side** ([src/cl_main.c](src/cl_main.c)):
- `CL_ConnectionlessPacket()` - Handles challenge/connection responses (line 1489)
- `Netchan_Setup()` - Establishes reliable channel (line 1583)
- `CL_ReadPackets()` - Main packet receiving loop (line 1704)
- `CL_SendCmd()` - Sends user commands ([src/cl_input.c#L1047](src/cl_input.c#L1047))

**Server Side** ([src/sv_main.c](src/sv_main.c)):
- `SV_ReadPackets()` - Receives client packets (line 2931)
- `SVC_DirectConnect()` - Handles new connections
- `SV_SendClientMessages()` - Transmits to all clients

### Protocol Extensions

The codebase supports extensive protocol extensions from FTE and ZQuake:

```c
// From src/qwprot/src/protocol.h
#define PROTOCOL_VERSION_FTE    (('F'<<0) + ('T'<<8) + ('E'<<16) + ('X' << 24))
#define PROTOCOL_VERSION_FTE2   (('F'<<0) + ('T'<<8) + ('E'<<16) + ('2' << 24))

// Key extensions:
#define FTE_PEXT_TRANS              0x00000008  // .alpha support
#define FTE_PEXT_ENTITYDBL          0x00002000  // 1024 entities
#define FTE_PEXT_FLOATCOORDS        0x00008000  // Float precision coordinates
#define FTE_PEXT_CHUNKEDDOWNLOADS   0x20000000  // Fast downloads
#define FTE_PEXT2_VOICECHAT         0x00000002  // Voice chat
```

---

## 4. Client-Side Prediction

### Location

Client-side prediction is implemented in [src/cl_pred.c](src/cl_pred.c).

### How It Works

The prediction system re-runs player movement locally to hide network latency:

```c
// From cl_pred.c
void CL_PredictUsercmd (player_state_t *from, player_state_t *to, usercmd_t *u) {
    // Split long moves
    if (u->msec > 50) {
        // Recursive subdivision
    }
    
    VectorCopy (from->origin, pmove.origin);
    VectorCopy (u->angles, pmove.angles);
    VectorCopy (from->velocity, pmove.velocity);
    
    // Run the shared player movement code
    PM_PlayerMove();
    
    VectorCopy (pmove.origin, to->origin);
}
```

### Prediction Pipeline

1. **Input Capture**: User commands are timestamped and stored
2. **Command Send**: Commands sent to server with sequence numbers
3. **Local Prediction**: Same physics (`PM_PlayerMove`) run locally
4. **Server Correction**: Server sends authoritative positions
5. **Reconciliation**: Client reconciles predicted vs actual state

### Controlling Prediction

```c
cvar_t cl_nopred = {"cl_nopred", "0"};           // Disable all prediction
cvar_t cl_predict_players = {"cl_predict_players", "1"};  // Predict other players
cvar_t cl_predict_half = {"cl_predict_half", "0"};        // Half-prediction
```

### Non-Predicted Mode

Setting `cl_nopred 1` disables prediction entirely. The client will display positions exactly as received from the server, resulting in visible latency but potentially useful for debugging or demos.

---

## 5. Player Limits

### The 32 Player Limit

The maximum client count is defined in the protocol, not as a simple constant:

```c
// Implicit in protocol - player numbers are 5 bits in some messages
#define MAX_CLIENTS 32  // (Not explicitly defined - derived from protocol)
```

The limit manifests throughout the codebase:

```c
// From src/cl_parse.c
if (slot >= MAX_CLIENTS)
    Host_Error ("CL_ParseServerMessage: svc_updateuserinfo > MAX_CLIENTS");

// From src/snd_voip.c
SpeexBits decbits[MAX_CLIENTS];
void *decoder[MAX_CLIENTS];
```

### Why 32?

1. **Protocol Encoding**: Player IDs encoded in 5 bits in several message types
2. **Entity Slots**: Entities 1-32 reserved for player edicts
3. **Bit Masks**: Player flags often stored as 32-bit masks
4. **Legacy Compatibility**: QuakeWorld protocol baked this in

### Original NetQuake: 16 Players

NetQuake used 4 bits for player numbers (0-15), limiting to 16 players.

### Increasing to 64 or 128

This would require **substantial protocol changes**:

| Area | Required Changes |
|------|------------------|
| **Protocol Messages** | Widen player ID fields from 5 to 6-7 bits in `svc_updatefrags`, `svc_updateping`, etc. |
| **Entity Reservation** | Change entity allocation to reserve more slots for players |
| **Fixed Arrays** | Update all `[MAX_CLIENTS]` arrays throughout codebase |
| **Bitmasks** | Replace 32-bit player masks with 64/128-bit alternatives |
| **Client Compatibility** | Would break compatibility with existing clients/servers |

### Specific Code Changes Needed

```c
// Arrays to resize:
player_state_t oldplayerstates[MAX_CLIENTS];  // cl_parse.c
SpeexBits decbits[MAX_CLIENTS];               // snd_voip.c
autoid_player_t autoids[MAX_CLIENTS];         // hud_autoid.c

// Protocol message encoding (sv_send.c, cl_parse.c):
// Player numbers embedded in message bytes - need format changes
```

**Estimated Effort**: Major undertaking requiring protocol version bump and extensive testing.

---

## 6. Bot Implementation

### Location

Bot support is provided through the PR2 (extended progs) system, primarily in:
- [src/sv_phys.c](src/sv_phys.c) - `SV_RunBots()` (line 1025)
- [src/pr2_cmds.c](src/pr2_cmds.c) - Bot creation/control functions

### Do Bots Use Client Slots?

**Yes.** Bots occupy standard client slots:

```c
// From src/server.h
typedef struct client_s {
    sv_client_state_t state;
    int isBot;           // Flag indicating this is a bot
    usercmd_t botcmd;    // Bot movement commands
    // ... same structure as regular clients
} client_t;
```

### Bot Lifecycle

```c
// From pr2_cmds.c - Bot spawn
newcl->isBot = 1;

// From sv_phys.c - Bot think
void SV_RunBots(void) {
    for (i = 0, cl = svs.clients; i < MAX_CLIENTS; i++, cl++) {
        if (cl->state == cs_free) continue;
        if (!cl->isBot) continue;
        
        sv_client = cl;
        sv_player = cl->edict;
        
        SV_PreRunCmd();
        SV_RunCmd(&cl->botcmd, false, false);
        SV_PostRunCmd();
    }
}
```

### Implications

- Bots count against `maxclients`
- Limited to the same MAX_CLIENTS (32) as regular players
- Bot AI is implemented in QuakeC (typically via KTX mod)
- Network bandwidth not consumed for bots (no actual packets)

---

## 7. Map & Entity Limits

### Entity Limits

```c
// From src/common.h
#define CL_MAX_EDICTS           2048    // Client-side entity limit
#define MAX_EDICTS              2048    // Server-side maximum
#define MAX_EDICTS_SAFE         512     // Conservative limit for compatibility
```

### Why These Limits?

1. **Protocol Encoding**: Entity numbers encoded in delta messages - 11 bits allows 2048
2. **Memory**: Each edict consumes memory for state and variables
3. **Network**: More entities = more bandwidth for updates

### FTE Extensions for Higher Limits

```c
#define FTE_PEXT_ENTITYDBL    0x00002000  // Double to 1024
#define FTE_PEXT_ENTITYDBL2   0x00004000  // Up to 2048
```

### Map Size Limits

Map geometry limits come from the BSP format:

| Limit | Value | Notes |
|-------|-------|-------|
| Coordinate Range | ±4096 units (standard) | `FTE_PEXT_FLOATCOORDS` extends this |
| Max Leafs | 32767 | BSP leaf node limit (signed 16-bit) |
| Max Faces/Surfaces | 65535 | Per-model surface limit (unsigned 16-bit) |
| Max Nodes | 32767 | BSP node limit (signed 16-bit for children) |
| Max Clipnodes | 32767 | Collision hull nodes |
| Max Lightmaps | 4 | Per surface (hardcoded) |

#### Extending Leafs/Surfaces to 32-bit: BSP2 Format

**Good news: This already exists!** ezQuake supports the **BSP2** format which uses 32-bit values:

```c
// From src/bspfile.h
#define Q1_BSPVERSION     29   // Original Quake BSP
#define Q1_BSPVERSION29a  (('2') + ('P' << 8) + ('S' << 16) + ('B' << 24))  // "2PSB" - RMQ extended
#define Q1_BSPVERSION2    (('B') + ('S' << 8) + ('P' << 16) + ('2' << 24))  // "BSP2" - Full 32-bit
```

**BSP2 Structure Differences:**

| Structure | Standard BSP (v29) | BSP2 |
|-----------|-------------------|------|
| `dnode_t.children` | `short[2]` | `int[2]` |
| `dnode_t.firstface` | `unsigned short` | `unsigned int` |
| `dnode_t.numfaces` | `unsigned short` | `unsigned int` |
| `dnode_t.mins/maxs` | `short[3]` | `float[3]` |
| `dleaf_t.firstmarksurface` | `unsigned short` | `unsigned int` |
| `dface_t.planenum` | `short` | `int` |
| `dface_t.numedges` | `short` | `int` |
| `dedge_t.v` | `unsigned short[2]` | `unsigned int[2]` |

**ezQuake BSP2 Support** (from [src/r_brushmodel_load.c](src/r_brushmodel_load.c) and [src/cmodel.c](src/cmodel.c)):

```c
// Detection and loading
if (mod->bspversion == Q1_BSPVERSION2 || mod->bspversion == Q1_BSPVERSION29a) {
    Mod_LoadEdgesBSP2(loadmodel, l, mod_base);
    Mod_LoadMarksurfacesBSP2(loadmodel, l, mod_base);
    Mod_LoadFacesBSP2(loadmodel, l, mod_base, bspx_header);
    Mod_LoadNodesBSP2(loadmodel, l, mod_base);
    // etc.
}
```

**Consequences of BSP2:**

| Aspect | Impact |
|--------|--------|
| **Map Compiler** | Need BSP2-capable compiler (ericw-tools, TyrUtils) |
| **Compatibility** | Only BSP2-aware engines can load these maps |
| **File Size** | ~10-20% larger BSP files due to wider integers |
| **Memory** | Slightly more RAM for node/leaf structures |
| **Coordinate Range** | BSP2 also supports float mins/maxs (±millions of units) |
| **Performance** | Negligible - same algorithms, just wider data |

**Current ezQuake BSP2 Limits (practical):**

| Limit | BSP2 Value | Practical Constraint |
|-------|------------|---------------------|
| Leafs | 2³¹ (~2 billion) | Memory/visibility data |
| Faces | 2³² (~4 billion) | GPU memory for lightmaps |
| Nodes | 2³¹ | Memory |
| Vertices | 2³² | GPU vertex buffers |

#### Increasing MAXLIGHTMAPS Per Surface

The current limit is 4 lightmaps per surface:

```c
// From src/bspfile.h
#define MAXLIGHTMAPS    4

typedef struct {
    short       planenum;
    // ...
    byte        styles[MAXLIGHTMAPS];  // Which light styles affect this surface
    int         lightofs;              // Offset into lightmap data
} dface_t;
```

**How Lightmaps Work:**

Each surface can be lit by up to 4 different "light styles" simultaneously. A light style is an animation pattern (like flickering lights). The `styles[]` array stores which style indices affect this surface, and the lightmap data contains separate lighting for each style that gets blended at runtime:

```c
// From src/r_lightmaps.c - Runtime blending
for (maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++) {
    scale = d_lightstylevalue[surf->styles[maps]];
    surf->cached_light[maps] = scale;
    
    bl = blocklights;
    for (i = 0; i < blocksize; i++) {
        *bl++ += lightmap[i] * scale;  // Accumulate contribution
    }
    lightmap += blocksize;  // Move to next style's data
}
```

**Can We Increase to 8, 16, 32, or 64?**

Technically possible, but with significant consequences:

| Change | Effort | Consequences |
|--------|--------|--------------|
| **4 → 8** | Medium | Double lightmap data size per surface, BSP format change |
| **4 → 16** | Medium | 4x lightmap data, significant memory increase |
| **4 → 32** | Hard | 8x lightmap data, may exceed GPU texture limits |
| **4 → 64** | Very Hard | 16x data, impractical memory usage |

**Required Code Changes for MAXLIGHTMAPS Increase:**

1. **BSP Format** ([src/bspfile.h](src/bspfile.h)):
   ```c
   #define MAXLIGHTMAPS    8  // or higher
   // All dface_t structures change size
   ```

2. **Surface Structure** ([src/gl_model.h](src/gl_model.h)):
   ```c
   typedef struct msurface_s {
       byte    styles[MAXLIGHTMAPS];
       int     cached_light[MAXLIGHTMAPS];  // Cache array grows
       // ...
   } msurface_t;
   ```

3. **Map Compiler**: Must output new format with more style slots

4. **Light Tool**: Must compute and store additional lightmap layers

5. **All Loops**: Every `for (maps = 0; maps < MAXLIGHTMAPS; ...)` works automatically

**Memory Impact Analysis:**

For a typical map with 10,000 surfaces, average 64x64 lightmap per surface:

| MAXLIGHTMAPS | Lightmap Data Size | Notes |
|--------------|-------------------|-------|
| 4 (current) | ~50 MB | Standard |
| 8 | ~100 MB | Noticeable |
| 16 | ~200 MB | GPU memory pressure |
| 32 | ~400 MB | Exceeds many GPU limits |
| 64 | ~800 MB | Impractical for most hardware |

**Practical Limit:**

- **8 lightmaps**: Reasonable for modern hardware, doubles flexibility
- **16 lightmaps**: Possible but diminishing returns - how many flickering lights do you really need overlapping one surface?
- **Beyond 16**: Mostly academic - you'd run out of light styles (64 total) before you'd usefully need this many per surface

**Alternative Approach:**

Rather than increasing MAXLIGHTMAPS, consider:
- More light styles globally (currently 64, could increase)
- Better light style assignment in map compiler
- Real-time lighting for dynamic effects instead of baked styles

### Other Asset Limits

```c
// From src/common.h
#define MAX_MODELS          4096    // Model precache limit (extended)
#define MAX_SOUNDS          256     // Sound precache limit
#define MAX_LIGHTSTYLES     64      // Dynamic light styles
#define MAX_STATIC_ENTITIES 2048    // func_illusionary, etc.
```

---

## 8. Asset Downloading

### Download Protocol

Downloads are handled via the `svc_download` message type:

```c
// From src/qwprot/src/protocol.h
#define svc_download  41  // [short] size [size bytes]
```

### Server-Side ([src/sv_user.c](src/sv_user.c))

```c
cvar_t allow_download = {"allow_download", "1"};
cvar_t allow_download_skins = {"allow_download_skins", "1"};
cvar_t allow_download_models = {"allow_download_models", "1"};
cvar_t allow_download_sounds = {"allow_download_sounds", "1"};
cvar_t allow_download_maps = {"allow_download_maps", "1"};
```

### Client-Side ([src/cl_parse.c](src/cl_parse.c))

```c
void CL_RequestNextDownload(void) {
    // Sequentially request needed files
    // Maps → Models → Sounds → Skins
}
```

### Chunked Downloads (FTE Extension)

```c
#define FTE_PEXT_CHUNKEDDOWNLOADS  0x20000000
```

This extension significantly improves download speed by:
- Allowing multiple chunks in flight
- Better handling of packet loss
- Parallel download capability

### Can Clients Download Mid-Game?

**Limited.** The download system is designed for connection-time precache:

1. **Map Changes**: Require reconnection for new BSP
2. **Models/Sounds**: Can technically be requested but:
   - Must be precached on server
   - Client needs to know to request them
   - No built-in "hot reload" mechanism

### Triggering Downloads

To add downloads mid-game would require:
1. Server sending `svc_stufftext` with download command
2. Client handling new precache indices
3. Proper ordering to avoid rendering missing assets

**Not a standard feature** - would need custom implementation.

---

## 9. Time Quantization & Rendering Loops

### Server Timing

```c
// From src/sv_main.c
cvar_t sv_mintic = {"sv_mintic", "0.013"};  // ~77 FPS max
cvar_t sv_maxtic = {"sv_maxtic", "0.1"};    // 10 FPS min
cvar_t sv_maxfps = {"maxfps", "77", CVAR_SERVERINFO};
```

### Server Frame ([src/sv_main.c#L3226](src/sv_main.c#L3226))

```c
void SV_Frame(double time) {
    // 1. Update timing
    if (!sv.paused) {
        realtime += time;
        sv.time += time;
    }
    
    // 2. Check client timeouts
    SV_CheckTimeouts();
    
    // 3. Read incoming packets
    SV_ReadPackets();
    
    // 4. Run physics
    if (!sv.paused) {
        SV_Physics();
        SV_RunBots();
    }
    
    // 5. Send updates to clients
    SV_SendClientMessages();
    
    // 6. Send MVD data
    SV_SendDemoMessage();
    
    // 7. Master server heartbeat
    Master_Heartbeat();
}
```

### Client Timing

The client has independent physics and rendering rates:

```c
// From src/cl_main.c
cvar_t cl_independentPhysics = {"cl_independentPhysics", "1"};
```

### Client Frame ([src/cl_main.c#L2433](src/cl_main.c#L2433))

```c
void CL_Frame(double time) {
    // 1. Accumulate time
    extratime += time;
    if (extratime < minframetime) return;
    
    // 2. Physics frame (may run independently)
    if (cl_independentPhysics.value) {
        if (extraphysframetime >= minphysframetime) {
            physframe = true;
            // Run physics this frame
        }
    }
    
    // 3. Update game time
    cls.realtime += cls.frametime;
    cl.time += cls.frametime;
    
    // 4. Read network packets
    CL_ReadPackets();
    
    // 5. Run prediction
    if (physframe) {
        CL_PredictMove(true);
    } else {
        CL_PredictMove(false);
    }
    
    // 6. Update entities
    CL_EmitEntities();
    
    // 7. Render
    SCR_UpdateScreen();
    
    // 8. Audio
    S_Update();
    
    // 9. Send commands
    CL_SendCmd();
}
```

### Host Frame Integration ([src/host.c#L473](src/host.c#L473))

```c
void Host_Frame(double time) {
    curtime += time;
    CL_Frame(time);  // CL_Frame internally calls SV_Frame for listen servers
}
```

---

## 10. Physics System

### Location

| File | Purpose |
|------|---------|
| [src/sv_phys.c](src/sv_phys.c) | Server-side entity physics |
| [src/pmove.c](src/pmove.c) | Player movement (shared) |
| [src/pmovetst.c](src/pmovetst.c) | Player collision testing |

### Movement Types

```c
// From sv_phys.c
switch ((int)ent->v->movetype) {
    case MOVETYPE_PUSH:    SV_Physics_Pusher(ent);  break;  // Doors, plats
    case MOVETYPE_NONE:    SV_Physics_None(ent);    break;  // Static
    case MOVETYPE_NOCLIP:  SV_Physics_Noclip(ent);  break;  // No collision
    case MOVETYPE_STEP:    SV_Physics_Step(ent);    break;  // Monsters
    case MOVETYPE_TOSS:
    case MOVETYPE_BOUNCE:
    case MOVETYPE_FLY:
    case MOVETYPE_FLYMISSILE:
        SV_Physics_Toss(ent);  break;  // Projectiles
}
```

### Physics Variables

```c
cvar_t sv_gravity         = {"sv_gravity", "800"};
cvar_t sv_maxvelocity     = {"sv_maxvelocity", "2000"};
cvar_t sv_friction        = {"sv_friction", "4"};
cvar_t sv_stopspeed       = {"sv_stopspeed", "100"};
cvar_t sv_accelerate      = {"sv_accelerate", "10"};
cvar_t sv_airaccelerate   = {"sv_airaccelerate", "10"};
```

### Physics Frame ([src/sv_phys.c](src/sv_phys.c))

```c
void SV_Physics(void) {
    // 1. Start frame (call QuakeC StartFrame)
    SV_ProgStartFrame(false);
    
    // 2. Run all entities
    for (i = 0; i < sv.num_edicts; i++) {
        SV_RunEntity(ent);
    }
    
    // 3. Update force_retouch
    if (PR_GLOBAL(force_retouch))
        PR_GLOBAL(force_retouch)--;
}
```

### Client Update Flow

1. Server runs `SV_Physics()` each tick
2. Entity states delta-encoded in `SV_WriteEntitiesToClient()`
3. Client receives via `CL_ParsePacketEntities()`
4. Client interpolates between received states
5. Prediction overlays local player movement

---

## 11. Shared Client/Server Code

### Key Shared Modules

| Module | Purpose | Used By |
|--------|---------|---------|
| [src/pmove.c](src/pmove.c) | Player movement physics | Both |
| [src/pmovetst.c](src/pmovetst.c) | Collision detection | Both |
| [src/cmodel.c](src/cmodel.c) | BSP collision model | Both |
| [src/mathlib.c](src/mathlib.c) | Vector/matrix math | Both |
| [src/cmd.c](src/cmd.c) | Command parsing | Both |
| [src/cvar.c](src/cvar.c) | Console variables | Both |
| [src/net_chan.c](src/net_chan.c) | Network channels | Both |
| [src/common.c](src/common.c) | Utility functions | Both |

### Conditional Compilation

```c
// Typical pattern
#ifdef SERVERONLY
    #include "qwsvdef.h"
#else
    #include "quakedef.h"
    #include "pmove.h"
#endif
```

### The pmove System

The player movement code is **critical shared code** - identical physics on client and server enables prediction:

```c
// Same function called by both:
// - Client: CL_PredictUsercmd() in cl_pred.c
// - Server: SV_RunCmd() in sv_user.c

int PM_PlayerMove(void) {
    // Unified player movement
    // Handles ground detection, jumping, friction, etc.
}
```

---

## 12. Console System

### Console Implementation

The console is implemented in [src/console.c](src/console.c) - a **client-only** feature:

```c
// Only compiled for client
// Uses rendering, key input, etc.
```

### Command System ([src/cmd.c](src/cmd.c))

The command system is shared but with separated command sets:

```c
// Command registration
void Cmd_AddCommand(char *cmd_name, xcommand_t function);
```

### Command Buffers

```c
cbuf_t cbuf_main;       // Main command buffer
cbuf_t cbuf_svc;        // Server command buffer
cbuf_t cbuf_safe;       // Safe (filtered) commands
cbuf_t cbuf_server;     // Server-side buffer
```

### Server Console

Dedicated servers have a simpler text-mode console:
- No graphics
- Direct stdin/stdout
- Implemented in [src/sv_sys_unix.c](src/sv_sys_unix.c) and [src/sv_sys_win.c](src/sv_sys_win.c)

```c
void SV_GetConsoleCommands(void) {
    // Read from stdin for dedicated servers
}
```

---

## 13. QuakeC Virtual Machine

### Architecture

ezQuake supports two QuakeC execution modes:

| Mode | Files | Description |
|------|-------|-------------|
| **PR1** (Classic) | [pr_exec.c](src/pr_exec.c), [pr_edict.c](src/pr_edict.c), [pr_cmds.c](src/pr_cmds.c) | Traditional QuakeC bytecode |
| **PR2** (Extended) | [pr2_exec.c](src/pr2_exec.c), [pr2_cmds.c](src/pr2_cmds.c) | QVM/Native module support |

### QuakeC Execution

```c
// From src/pr_exec.c
void PR_ExecuteProgram(func_t fnum) {
    f = &pr_functions[fnum];
    
    while (1) {
        st = &pr_statements[s];
        
        switch (st->op) {
            case OP_ADD_F: c->_float = a->_float + b->_float; break;
            case OP_MUL_V: /* vector dot product */ break;
            case OP_CALL0...OP_CALL8: /* function call */ break;
            // ... ~60 opcodes
        }
    }
}
```

### Built-in Functions

C functions exposed to QuakeC via the builtin table:

```c
// From src/pr_cmds.c
void PF_makevectors(void) {
    AngleVectors(G_VECTOR(OFS_PARM0), PR_GLOBAL(v_forward), 
                 PR_GLOBAL(v_right), PR_GLOBAL(v_up));
}

void PF_setorigin(void) {
    edict_t *e = G_EDICT(OFS_PARM0);
    float *org = G_VECTOR(OFS_PARM1);
    VectorCopy(org, e->v->origin);
    SV_LinkEdict(e, false);
}
```

### Data Exchange

```c
// C → QuakeC (setting globals)
pr_global_struct->self = EDICT_TO_PROG(ent);
pr_global_struct->time = sv.time;

// QuakeC → C (reading return values)
float result = G_FLOAT(OFS_RETURN);
edict_t *ent = G_EDICT(OFS_RETURN);
char *str = G_STRING(OFS_RETURN);
```

### When QuakeC Runs

QuakeC is invoked at specific points in the server loop:

```c
// Start of each physics frame
PR_ExecuteProgram(PR_GLOBAL(StartFrame));

// Entity think functions
PR_EdictThink(ent->v->think);

// Touch callbacks
PR_EdictTouch(e1->v->touch);

// Client events
PR_ExecuteProgram(PR_GLOBAL(ClientConnect));
PR_ExecuteProgram(PR_GLOBAL(PlayerPreThink));
PR_ExecuteProgram(PR_GLOBAL(PlayerPostThink));
```

### PR2 (Extended) System

PR2 adds support for native modules via the Q3-style VM:

```c
// From src/vm.h
typedef enum {
    VMI_NONE,       // Classic PR1
    VMI_NATIVE,     // Native .so/.dll
    VMI_BYTECODE,   // QVM interpreted
    VMI_COMPILED    // QVM JIT compiled
} vmInterpret_t;
```

---

## 14. Extending QuakeC

### Client-Side QuakeC (CSQC)

**ezQuake does NOT support CSQC.** There is no client-side QuakeC execution:

```c
// No CSQC files found in codebase
// grep -r "CSQC" returns no results
```

This is a QuakeWorld limitation - CSQC was developed later for engines like DarkPlaces and FTE.

### Adding New Builtins (Server)

To add a new QuakeC builtin:

1. **Define the function** in [src/pr_cmds.c](src/pr_cmds.c):
```c
void PF_myfunction(void) {
    char *arg1 = G_STRING(OFS_PARM0);
    float arg2 = G_FLOAT(OFS_PARM1);
    
    // Do something
    
    G_FLOAT(OFS_RETURN) = result;
}
```

2. **Register in builtin table**:
```c
builtin_t pr_builtins[] = {
    // ... existing builtins
    PF_myfunction,  // At a specific index
};
```

3. **Document for QC usage**:
```qc
float(string s, float f) myfunction = #XXX;
```

### Adding HTTP Requests to QuakeC

This is theoretically possible but non-trivial:

```c
// Hypothetical implementation in pr_cmds.c
void PF_http_request(void) {
    char *url = G_STRING(OFS_PARM0);
    char *callback_func = G_STRING(OFS_PARM1);
    
    // Problem: HTTP is asynchronous, QC is synchronous
    // Options:
    // 1. Block (bad - freezes server)
    // 2. Queue request, call callback later
    // 3. Use existing central.c pattern
}
```

The `central.c` module already does HTTP for master server communication - could be extended.

### Challenges

1. **Async Nature**: HTTP responses arrive asynchronously
2. **Memory Management**: QC strings have specific lifetime
3. **Security**: Arbitrary HTTP from mods is risky
4. **Thread Safety**: Would need careful integration

---

## 15. Dynamic Entity Management

### Can Entities Be Added Mid-Game?

**Yes, with limitations.**

Entities can be spawned/removed during gameplay:

```c
// QuakeC spawns entities
edict_t *ED_Alloc(void);  // Allocate new entity
void ED_Free(edict_t *ed);  // Remove entity
```

### Model/Sound Precaching

Assets **must be precached at map load**:

```c
// From pr_cmds.c
void PF_precache_model(void) {
    if (sv.state != ss_loading)
        SV_Error("precache_model: called during gameplay");
    // ...
}
```

### What Can Change Mid-Game

| Can Change | Cannot Change |
|------------|---------------|
| Entity spawn/remove | New model precaches |
| Entity model (from precached) | New sound precaches |
| Entity properties | Map geometry |
| Dynamic lights | BSP structure |

### Workarounds

1. **Precache Pool**: Precache many models at start, use subsets
2. **Model Replacement**: Point entity to different precached model
3. **Custom Protocol**: Send svc_stufftext to trigger reconnect for major changes

---

## 16. Embedding Scripting Interpreters

### Embedding Python/Lua - Conceptual Approach

This is feasible and has been done in other Quake engines (e.g., Xonotic with Lua).

### Integration Points

```c
// Key areas to hook:

// 1. Initialization (sv_init.c or host.c)
void SV_SpawnServer(...) {
    // After progs load
    Script_Init();
    Script_LoadServerScripts();
}

// 2. Frame hook (sv_main.c)
void SV_Frame(double time) {
    // Before or after physics
    Script_Frame(time);
}

// 3. Event hooks (various)
void SV_ClientConnect(...) {
    Script_OnClientConnect(client);
}
```

### Exposing Game State

```c
// Python/Lua would need bindings to:
typedef struct {
    // Read/write entity fields
    vec3_t origin;
    vec3_t velocity;
    float health;
    // ...
} script_entity_t;

// Example Python integration pseudocode
static PyObject* py_get_entity(PyObject *self, PyObject *args) {
    int num;
    PyArg_ParseTuple(args, "i", &num);
    
    edict_t *ed = EDICT_NUM(num);
    return Entity_ToPython(ed);
}
```

### Challenges

1. **Memory**: Python/Lua runtimes add significant overhead
2. **Performance**: Scripting slower than compiled C
3. **Thread Safety**: Quake is single-threaded; scripts must be too
4. **State Sync**: Scripts need safe access to game structures
5. **Build Complexity**: Linking Python/Lua adds dependencies

### Recommended Approach

```
┌─────────────────────────────────────────────────────────┐
│ Lua (lighter weight, simpler embedding)                 │
│ ├── luajit for performance                              │
│ ├── Sol2 or similar C++ wrapper                         │
│ └── Expose entity table, console commands               │
├─────────────────────────────────────────────────────────┤
│ Python (more powerful, heavier)                         │
│ ├── Embed via Python C API                              │
│ ├── Use cffi/ctypes for bindings                        │
│ └── Consider subprocess model for isolation             │
└─────────────────────────────────────────────────────────┘
```

### Example Lua Hook

```c
// In sv_phys.c
void SV_Physics(void) {
    // Before standard physics
    lua_getglobal(L, "PrePhysics");
    lua_pushnumber(L, sv.time);
    lua_call(L, 1, 0);
    
    // Standard physics
    SV_ProgStartFrame(false);
    // ...
}
```

---

## 17. Threading Model

### Is ezQuake Single-Threaded?

**Mostly yes, but not entirely.**

### Main Thread

The core game loop (physics, rendering, networking) is single-threaded:

```c
// Main loop (sys_posix.c / sys_win.c)
while (1) {
    newtime = Sys_DoubleTime();
    time = newtime - oldtime;
    oldtime = newtime;
    
    Host_Frame(time);  // Everything happens here
}
```

### Background Threads

ezQuake uses threads for specific non-critical tasks:

```c
// From src/sys_sdl2.c
int Sys_CreateDetachedThread(int (*func)(void *), void *data) {
    SDL_Thread *thread = SDL_CreateThread(func, "ezquake_worker", data);
    if (thread) {
        SDL_DetachThread(thread);
        return 0;
    }
    return -1;
}
```

### Thread Usage

| Feature | Threading | Location |
|---------|-----------|----------|
| Server browser pings | Background thread | [EX_browser_ping.c](src/EX_browser_ping.c) |
| QTV list refresh | Background thread | [EX_browser_qtvlist.c](src/EX_browser_qtvlist.c) |
| Server list updates | Background thread | [EX_browser_net.c](src/EX_browser_net.c) |
| Demo capture | Background threads | Movie capture system |
| Mouse input | Optional thread | `in_mmt` cvar |
| HTTP requests | Background | [match_tools_challenge.c](src/match_tools_challenge.c) |

### Thread Safety

The codebase is NOT thread-safe in general:
- No mutexes around game state
- Background threads only touch isolated data
- Results marshalled back to main thread via queues

### Implications

1. **Cannot parallelize physics** without major refactoring
2. **Network processing is serialized** 
3. **Rendering is single-threaded**
4. **Adding threads requires careful isolation**

---

## Summary

ezQuake is a mature, feature-rich QuakeWorld client with embedded server capabilities. Key architectural highlights:

- **Server Code**: Full server included, buildable as dedicated or listen server
- **Protocol**: QuakeWorld with extensive FTE/ZQuake extensions
- **Limits**: 32 players, 2048 entities (protocol-bound)
- **QuakeC**: Classic PR1 interpreter + PR2 native/QVM support (server only)
- **Prediction**: Full client-side prediction via shared pmove code
- **Threading**: Primarily single-threaded with background workers for I/O
- **Extensibility**: New builtins feasible; scripting would require significant work

The codebase is well-organized but tightly coupled to original Quake's architecture, making fundamental changes (like player limits or CSQC) challenging but not impossible.

---

## 18. Scaling Entities to 65536 and Beyond

### Current Protocol Encoding

The QuakeWorld protocol encodes entity numbers in the first 16 bits of a packet entity update:

```c
// From src/qwprot/src/protocol.h (line 342)
// "the first 16 bits of a packetentities update holds 9 bits of entity 
// number and 7 bits of flags"
```

This means standard QuakeWorld supports only **512 entities** (9 bits = 0-511).

### FTE Extensions for Higher Entity Counts

ezQuake uses FTE protocol extensions to increase this:

```c
// From src/qwprot/src/protocol.h
#define U_FTE_ENTITYDBL    (1<<11)  // Entity number >= 512
#define U_FTE_ENTITYDBL2   (1<<12)  // Entity number >= 1024

// From src/sv_ents.c SV_WriteDelta()
if (to->number >= 512) {
    if (to->number >= 1024) {
        if (to->number >= 1024 + 512) {
            evenmorebits |= U_FTE_ENTITYDBL;
            required_extensions |= FTE_PEXT_ENTITYDBL;
        }
        evenmorebits |= U_FTE_ENTITYDBL2;
        required_extensions |= FTE_PEXT_ENTITYDBL2;
        if (to->number >= 2048)
            SV_Error("Entity number >= 2048");  // Hard limit!
    }
    else {
        evenmorebits |= U_FTE_ENTITYDBL;
        required_extensions |= FTE_PEXT_ENTITYDBL;
    }
}
```

### Current Hard Limit: 2048

The code explicitly errors at 2048 entities:

```c
// From src/common.h
#define MAX_EDICTS  2048  // can't encode more than this, see SV_WriteDelta
```

### Scaling to 65536 Entities

To support 65536 entities, you need **16 bits** for entity numbers. Here's what must change:

#### 1. Protocol Extension (New Flag)

```c
// Proposed new extension
#define FTE_PEXT_ENTITYDBL3    0x00008000  // Entity numbers 2048-65535
#define U_FTE_ENTITYDBL3       (1<<13)     // Third doubling flag
```

#### 2. Entity Encoding Changes ([src/sv_ents.c](src/sv_ents.c))

```c
// Modified SV_WriteDelta for 65536 entities
if (to->number >= 512) {
    if (to->number >= 1024) {
        if (to->number >= 2048) {
            if (to->number >= 65536)
                SV_Error("Entity number >= 65536");
            
            // New: Third doubling for 2048-65535
            evenmorebits |= U_FTE_ENTITYDBL | U_FTE_ENTITYDBL2 | U_FTE_ENTITYDBL3;
            required_extensions |= FTE_PEXT_ENTITYDBL3;
        }
        else if (to->number >= 1024 + 512) {
            evenmorebits |= U_FTE_ENTITYDBL;
        }
        evenmorebits |= U_FTE_ENTITYDBL2;
        required_extensions |= FTE_PEXT_ENTITYDBL2;
    }
    else {
        evenmorebits |= U_FTE_ENTITYDBL;
        required_extensions |= FTE_PEXT_ENTITYDBL;
    }
}

// Writing entity number - need MSG_WriteShort for full 16-bit
if (evenmorebits & U_FTE_ENTITYDBL3) {
    MSG_WriteShort(msg, to->number);  // Full 16-bit entity number
}
```

#### 3. Memory Structures ([src/common.h](src/common.h), [src/server.h](src/server.h))

```c
#define MAX_EDICTS  65536

// Server edict array becomes huge:
// sizeof(edict_t) ≈ 800 bytes × 65536 = ~50 MB just for edicts
```

#### 4. Client Entity Array ([src/client.h](src/client.h))

```c
// Client-side entity tracking
entity_t cl_entities[65536];      // ~4 MB
entity_state_t cl_baselines[65536]; // ~3 MB
```

### Consequences of 65536 Entities

| Aspect | Impact | Mitigation |
|--------|--------|------------|
| **Memory** | +50-60 MB server, +10 MB client | Modern systems handle this |
| **Per-frame processing** | 32× more entity checks | Spatial partitioning, PVS |
| **Network bandwidth** | Massive increase (see Section 20) | Delta compression, priority |
| **QuakeC globals** | More globals array space | PR1 may need patching |
| **PVS Calculation** | Much more to cull | Essential for performance |
| **Compatibility** | Breaks all old clients/servers | Requires version negotiation |

### Beyond 65536: Theoretical Limits

Going to **262144 (2¹⁸)** or **1 million** entities:

| Entity Count | Bits Needed | Feasibility |
|--------------|-------------|-------------|
| 65536 | 16 | Achievable with protocol work |
| 262144 | 18 | Custom protocol, serious bandwidth |
| 1M | 20 | Custom engine, dedicated arch |
| 16M | 24 | MMO-style, complete redesign |

For 1M+ entities, you'd need:
1. **Hierarchical entity management** - Only track entities in "areas"
2. **Interest management** - Clients only know about nearby entities
3. **Server sharding** - Different servers own different areas (see Section 21)
4. **Entity aggregation** - Groups of units as single logical entity

---

## 19. Model Technical Limits

### Alias Model (MDL) Limits

From [src/gl_model.h](src/gl_model.h):

```c
#define MAXALIASVERTS   2048    // Maximum vertices per model
#define MAXALIASFRAMES  256     // Maximum animation frames
#define MAXALIASTRIS    2048    // Maximum triangles per model
#define MAX_SKINS       32      // Maximum skin textures per model
```

### Triangle/Polygon Limits

| Limit | Value | Source |
|-------|-------|--------|
| **Max triangles** | 2048 | `MAXALIASTRIS` in gl_model.h |
| **Max vertices** | 2048 | `MAXALIASVERTS` in gl_model.h |
| **Vertex format** | 8-bit per axis | `trivertx_t { byte v[3]; }` in modelgen.h |
| **Position precision** | 256 steps per axis | Scaled by `hdr->scale` |

The triangle limit is enforced at load time:

```c
// From src/r_aliasmodel.c
if (pheader->numverts > MAXALIASVERTS) {
    // Error handling
}
```

### Animation Frame Limits

| Limit | Value | Notes |
|-------|-------|-------|
| **Max frames** | 256 | `MAXALIASFRAMES` in gl_model.h |
| **Frame storage** | Array of vertex snapshots | Each frame = numverts × 4 bytes |
| **Memory per model** | frames × verts × 4 bytes | 256 × 2048 × 4 = 2 MB max |

Frame data structure:

```c
// From src/modelgen.h
typedef struct {
    byte    v[3];              // Compressed position (8-bit per axis)
    byte    lightnormalindex;  // Index into precomputed normals table
} trivertx_t;
```

### Texture/Skin Limits

| Limit | Value | Notes |
|-------|-------|-------|
| **Max skins** | 32 | `MAX_SKINS` in gl_model.h |
| **Skin groups** | 4 sub-skins per group | For animated skins |
| **Texture size** | GPU-limited | Typically 256×256 original, up to 4096×4096 modern |
| **Format** | 8-bit paletted (original) | 32-bit RGBA with external textures |

From [src/gl_model.h](src/gl_model.h):

```c
typedef struct aliashdr_s {
    int          numskins;
    int          skinwidth;    // No hard limit, but typically 256-512
    int          skinheight;
    texture_ref  gl_texturenum[MAX_SKINS][4];  // [skin][subgroup]
    // ...
} aliashdr_t;
```

**GPU Texture Size Limit:**

```c
// From src/vid_common_gl.c
glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glConfig.gl_max_size_default);
// Typically 8192 or 16384 on modern GPUs
```

### UV Coordinate Calculation

Quake MDL format uses **integer UV coordinates** stored per-vertex:

```c
// From src/modelgen.h
typedef struct {
    int     onseam;  // Is vertex on skin seam?
    int     s;       // U coordinate (in skin pixels)
    int     t;       // V coordinate (in skin pixels)
} stvert_t;
```

**UV Transformation at Load Time** ([src/glc_aliasmodel_mesh.c](src/glc_aliasmodel_mesh.c)):

```c
// Convert integer pixel coords to normalized 0.0-1.0 UVs
s = stverts[k].s;
t = stverts[k].t;

// Handle back-side seam (duplicate vertices on wrap seam)
if (!triangles[besttris[0]].facesfront && stverts[k].onseam) {
    s += pheader->skinwidth / 2;  // Offset to back side of skin
}

// Normalize to 0.0-1.0 range
s = (s + 0.5) / pheader->skinwidth;
t = (t + 0.5) / pheader->skinheight;
```

**The Seam System:**

Original Quake models use a clever trick to reduce texture duplication:
1. Skin texture has front and back halves side-by-side
2. `onseam` flag marks vertices at the edge
3. When rendering back faces, UV is offset by half skin width
4. This allows seamless wrapping with one texture

### MD3 Model Support

ezQuake also supports MD3 format ([src/gl_aliasmodel_md3.c](src/gl_aliasmodel_md3.c)) with higher limits:

| Limit | MD3 Value | Notes |
|-------|-----------|-------|
| **Triangles** | 8192 per surface | Multiple surfaces allowed |
| **Vertices** | 4096 per surface | 16-bit indices |
| **Frames** | 1024 | Higher than MDL |
| **UV precision** | 16-bit fixed point | More precise than MDL |

### Expanding Model Limits for RTS

For a massive RTS with many unit types:

```c
// Proposed expanded limits
#define MAXALIASVERTS   8192    // 4× current
#define MAXALIASFRAMES  1024    // 4× current  
#define MAXALIASTRIS    16384   // 8× current
#define MAX_SKINS       128     // 4× current (for factions, variations)
```

**Consequences:**

| Change | Memory Impact | Performance Impact |
|--------|--------------|-------------------|
| 8K verts | 4× vertex buffer | Negligible on modern GPU |
| 16K tris | 8× index buffer | Minor render cost |
| 1K frames | 4× animation data | ~8 MB per complex model |
| 128 skins | 4× texture slots | More VRAM usage |

---

## 20. Bandwidth Analysis & Scaling

### Per-Entity Network Cost

Each entity update in the QuakeWorld protocol consists of:

```c
// From src/sv_ents.c SV_WriteDelta()
// Minimum update (entity just moves):
MSG_WriteShort(msg, entitynum | flags);  // 2 bytes: entity + base flags
// Optional additional bytes based on what changed:
if (bits & U_MOREBITS)    MSG_WriteByte(msg, bits);      // +1 byte
if (bits & U_MODEL)       MSG_WriteByte(msg, modelindex); // +1 byte
if (bits & U_FRAME)       MSG_WriteByte(msg, frame);      // +1 byte
if (bits & U_COLORMAP)    MSG_WriteByte(msg, colormap);   // +1 byte
if (bits & U_SKIN)        MSG_WriteByte(msg, skinnum);    // +1 byte
if (bits & U_EFFECTS)     MSG_WriteByte(msg, effects);    // +1 byte
if (bits & U_ORIGIN1)     MSG_WriteCoord(msg, origin[0]); // +2 bytes
if (bits & U_ANGLE1)      MSG_WriteAngle(msg, angles[0]); // +1 byte
// ... up to 3 origins + 3 angles
```

**Typical Entity Update Sizes:**

| Scenario | Bytes | Notes |
|----------|-------|-------|
| **No change** | 0 | Delta compression skips unchanged |
| **Position only** | 8 | Header + 3 coords |
| **Position + angle** | 11 | + 3 angle bytes |
| **Full update** | 16-20 | All fields changed |
| **New entity (baseline)** | 20-24 | Full state required |

### Maximum Packet Size

```c
// From src/q_shared.h
#define MAX_MSGLEN   1450   // Max reliable message length
#define MAX_DATAGRAM 1450   // Max unreliable message length
```

This means ~72-180 entity updates per packet maximum.

### Bandwidth Scaling Formula

For `N` entities with `P` percent changing each frame at `T` tickrate:

$$
\text{Bandwidth (bytes/sec)} = N \times \frac{P}{100} \times \text{avg\_update\_size} \times T
$$

**Example Calculations (at 77 Hz server tick):**

| Entities | Active % | Avg Size | Bandwidth/Client | For 32 Clients |
|----------|----------|----------|------------------|----------------|
| 512 | 20% | 12 | 94 KB/s | 3 MB/s |
| 2048 | 20% | 12 | 378 KB/s | 12 MB/s |
| 8192 | 10% | 12 | 756 KB/s | 24 MB/s |
| 65536 | 5% | 12 | 3 MB/s | 96 MB/s |

### Bandwidth Explosion Problem

As entity count increases linearly, bandwidth grows **superlinearly** due to:

1. **More entities in PVS**: Larger areas mean more visible entities
2. **More delta failures**: Cache pressure causes more full updates
3. **More clients**: Each client needs their own entity view
4. **Interaction effects**: More entities = more state changes

### Mitigation Strategies

#### 1. Aggressive PVS Culling

Only send entities the client can actually see:

```c
// Current approach in sv_ents.c
void SV_WriteEntitiesToClient(client_t *client, sizebuf_t *msg) {
    // Build PVS for client position
    VectorCopy(cl->edict->v->origin, org);
    pvs = SV_FatPVS(org);
    
    for (e = 1; e < sv.num_edicts; e++) {
        ent = EDICT_NUM(e);
        // Skip if not in PVS
        if (!SV_EdictInPVS(ent, pvs))
            continue;
        // ... send entity
    }
}
```

#### 2. Area-of-Interest (AOI) System

Send detailed updates for nearby entities, less detail for distant:

```c
// Proposed tiered update system
typedef enum {
    UPDATE_FULL,      // < 500 units: every tick
    UPDATE_MEDIUM,    // 500-1500: every 3rd tick
    UPDATE_LOW,       // 1500-3000: every 10th tick
    UPDATE_MINIMAL    // > 3000: position only, every 20th tick
} update_priority_t;
```

#### 3. Entity Aggregation

Group many units into "squads" transmitted as single entities:

```c
// Proposed: Squad entity for 8 units
typedef struct {
    vec3_t  center;           // Squad center position
    byte    formation;        // Formation type
    byte    unit_count;       // How many in squad
    short   unit_offsets[8];  // Packed x,y offsets from center
    byte    unit_frames[8];   // Animation frames
} squad_state_t;  // ~25 bytes vs 8×16 = 128 bytes individual
```

#### 4. Snapshot Compression

Use delta-from-baseline more aggressively:

```c
// Proposed: Multi-level delta compression
entity_state_t baseline[MAX_EDICTS];      // Level 1: spawn state
entity_state_t last_acked[MAX_EDICTS];    // Level 2: last confirmed
entity_state_t predicted[MAX_EDICTS];     // Level 3: predicted state

// Send delta from whatever is most similar
delta_from = choose_best_baseline(ent, client);
```

### Practical Bandwidth Limits

| Target | Entity Count | Required Bandwidth | Feasibility |
|--------|-------------|-------------------|-------------|
| **LAN Play** | 8192 | 100 MB/s | ✓ Gigabit LAN |
| **Home Broadband** | 2048 | 5-10 MB/s | ✓ Modern connections |
| **Average Internet** | 512-1024 | 1-2 MB/s | ✓ Most users |
| **Mobile/Constrained** | 256 | 0.5 MB/s | ✓ With compression |
| **Massive (65K)** | 65536 | 100+ MB/s | ✗ Not viable single-server |

**Conclusion:** For 65536+ entities, **federated servers** are mandatory.

---

## 21. Federated Multi-Server Architecture

### The Vision

A constellation of QuakeWorld servers that:
1. Share game state across server boundaries
2. Allow clients to seamlessly move between servers
3. Each server owns a "zone" or map
4. Bridge points connect zones for seamless crossover
5. Community members can run servers and federate into the world

### Existing Foundation: QTV

ezQuake already has QTV (QuakeTV) which demonstrates multi-server concepts:

```c
// From src/qtv.h
#define QTV_VERSION  1.0f

// QTV acts as a relay between servers and spectating clients
// Key concepts we can build on:
// - Server-to-server connections
// - State forwarding
// - Client handoff (spectator joins game server)
```

### Proposed Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    FEDERATION COORDINATOR                        │
│        (Lightweight service tracking all servers)               │
└─────────────────────────────────────────────────────────────────┘
         │              │              │              │
         ▼              ▼              ▼              ▼
    ┌─────────┐   ┌─────────┐   ┌─────────┐   ┌─────────┐
    │ Zone A  │◄─►│ Zone B  │◄─►│ Zone C  │◄─►│ Zone D  │
    │ Server  │   │ Server  │   │ Server  │   │ Server  │
    │(Castle) │   │(Forest) │   │(Desert) │   │(Ocean)  │
    └─────────┘   └─────────┘   └─────────┘   └─────────┘
         ▲              ▲              ▲              ▲
         │              │              │              │
    ┌────┴────┐   ┌────┴────┐   ┌────┴────┐   ┌────┴────┐
    │Clients  │   │Clients  │   │Clients  │   │Clients  │
    │0-50     │   │0-50     │   │0-50     │   │0-50     │
    └─────────┘   └─────────┘   └─────────┘   └─────────┘
```

### Server-to-Server Protocol

```c
// Proposed inter-server messages
typedef enum {
    // Coordination
    ISM_HELLO,           // Register with coordinator
    ISM_ZONE_INFO,       // Advertise zone boundaries
    ISM_NEIGHBOR_SYNC,   // Establish neighbor relationship
    
    // Entity Handoff
    ISM_ENTITY_ENTER,    // Entity approaching border
    ISM_ENTITY_HANDOFF,  // Transfer entity ownership
    ISM_ENTITY_ACK,      // Confirm receipt
    
    // State Sync
    ISM_BORDER_ENTITIES, // Entities near shared border (for visibility)
    ISM_GLOBAL_EVENT,    // World-wide event (boss spawn, etc.)
    
    // Player Transfer
    ISM_PLAYER_PREREQ,   // Player approaching, prepare for handoff
    ISM_PLAYER_TRANSFER, // Full player state transfer
    ISM_PLAYER_READY,    // New server ready, tell client to reconnect
} inter_server_msg_t;
```

### Seamless Zone Transition

When a client approaches a zone boundary:

```
Timeline of seamless transfer:

T+0ms:    Client at 95% of zone boundary
          Server A: ISM_PLAYER_PREREQ to Server B
          
T+50ms:   Server B: Creates ghost entity, reserves slot
          Server B: ISM_PLAYER_READY to Server A
          
T+100ms:  Client crosses boundary
          Server A: ISM_PLAYER_TRANSFER (full state) to Server B
          Server A: Sends svc_redirect to client
          
T+150ms:  Client receives redirect, initiates connect to Server B
          Client: Sends "reconnect <token>" to Server B
          
T+200ms:  Server B: Recognizes token, loads transferred state
          Server B: Sends svc_serverinfo with current state
          
T+300ms:  Client fully connected to Server B
          No perceptible interruption (just 200-300ms latency spike)
```

### Border Entity Visibility

For entities near zone boundaries, both servers need awareness:

```c
// Proposed border entity synchronization
typedef struct {
    int         entity_id;
    int         owner_server;       // Which server owns this entity
    vec3_t      position;
    vec3_t      velocity;
    byte        model_index;
    byte        frame;
    byte        flags;              // BORDER_ENT_HOSTILE, etc.
    float       last_update;
} border_entity_t;

// Each server maintains:
border_entity_t border_entities[MAX_BORDER_ENTITIES];

// Synced at 10-20 Hz for entities within 500 units of border
```

### Federation Coordinator Service

A lightweight service (not a game server) that:

```c
// Coordinator responsibilities
typedef struct federation_coordinator_s {
    // Registry of active servers
    server_info_t   servers[MAX_FEDERATION_SERVERS];
    int             num_servers;
    
    // Zone topology (which zones connect)
    zone_link_t     zone_links[MAX_ZONE_LINKS];
    
    // Global state
    int             global_player_count;
    int             global_entity_count;
    
    // Match/event coordination
    global_event_t  active_events[MAX_EVENTS];
} federation_coordinator_t;

// Protocol: Simple JSON/MessagePack over TCP
// Heartbeat: Servers ping every 10 seconds
// Bandwidth: Minimal (~1 KB/s per server)
```

### Map Bridge Points

Special trigger entities that initiate cross-server travel:

```c
// QuakeC for zone portal
void trigger_zoneportal_touch() {
    if (other.classname != "player")
        return;
    
    // Notify server of impending transfer
    string dest_server = self.target_server;  // "zone_b.example.com:27500"
    string dest_spawn = self.target_spawn;    // Spawn point name in dest map
    
    // Server-side builtin initiates transfer protocol
    sv_initiate_zone_transfer(other, dest_server, dest_spawn);
}
```

### Two Servers, Different Maps: Bridge Implementation

```
┌────────────────────────────────────────────────────────────────┐
│                        SERVER A                                 │
│                     Map: castle.bsp                            │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │                                                          │  │
│  │    [Castle Interior]                                    │  │
│  │                                                          │  │
│  │                              ╔══════════════════════╗   │  │
│  │                              ║ trigger_zoneportal   ║   │  │
│  │                              ║ target: forest.bsp   ║   │  │
│  │                              ║ dest: forest_entry   ║   │  │
│  │                              ╚══════════════════════╝   │  │
│  └─────────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────────┘
                              │
                    ISM_PLAYER_TRANSFER
                              │
                              ▼
┌────────────────────────────────────────────────────────────────┐
│                        SERVER B                                 │
│                     Map: forest.bsp                            │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │  ╔══════════════════════╗                               │  │
│  │  ║ info_player_start    ║                               │  │
│  │  ║ name: forest_entry   ║                               │  │
│  │  ╚══════════════════════╝                               │  │
│  │                                                          │  │
│  │                 [Forest Landscape]                       │  │
│  │                                                          │  │
│  └─────────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────────┘
```

### Community Federation

For community-run servers to join the federation:

```c
// Configuration for community server joining federation
// server.cfg:

// Register with coordinator
set sv_federation_coordinator "master.quake-rts.example.com:27000"
set sv_zone_name "player_castle_joe"
set sv_zone_type "player_base"  // Affects spawn rules, etc.

// Define border connections (can be dynamic)
set sv_zone_neighbor_1 "hub.quake-rts.example.com:27500"
set sv_zone_border_1_mins "-4096 3000 -512"
set sv_zone_border_1_maxs "-3500 4096 512"

// Trust settings
set sv_federation_trust "1"  // Accept player transfers
set sv_federation_entity_sync "1"  // Sync border entities
```

### State Synchronization Challenges

| Challenge | Solution |
|-----------|----------|
| **Clock Sync** | NTP + relative timestamps, 10ms tolerance |
| **Entity ID Conflicts** | Server prefix: `(server_id << 16) \| local_id` |
| **Authoritative State** | Owning server is authoritative; border sync is hints |
| **Trust** | Coordinator validates server registration |
| **Cheating** | Cross-reference player positions across servers |
| **Latency** | 100ms target for zone handoff |

---

## 22. Dynamic Media Loading

### Current Asset Loading

ezQuake already supports multiple image formats:

```c
// From src/image.h
byte *Image_LoadPNG(vfsfile_t *v, const char *path, ...);
byte *Image_LoadTGA(vfsfile_t *v, const char *path, ...);
byte *Image_LoadPCX(vfsfile_t *v, const char *path, ...);
byte *Image_LoadJPEG(vfsfile_t *v, const char *path, ...);
```

### Texture Loading Pipeline

```c
// From src/r_texture_load.c
// Format detection and loading
static struct {
    const char* extension;
    load_func_t loader;
} image_loaders[] = {
    { "tga", Image_LoadTGA },
    { "png", Image_LoadPNG },
    // jpeg, pcx, etc.
};

// Texture search paths
// 1. textures/<name>.<ext>
// 2. <mapname>/<name>.<ext>
// 3. Override packs
```

### Runtime Texture Loading

For RTS, we need to load textures during gameplay:

```c
// Proposed dynamic texture API
texture_ref R_LoadDynamicTexture(const char *name, const char *url);
void R_UnloadDynamicTexture(texture_ref tex);

// Usage in QuakeC (new builtin)
// float tex = loadtexture("units/custom_tank", "http://server/tank.png");
// setmodel(self, "progs/tank.mdl"); 
// self.skin_override = tex;
```

### Sound Loading

Current sound system:

```c
// From src/snd_main.c
#define MAX_SFX (MAX_SOUNDS*2)  // 512 sounds

// Precache at map load
S_PrecacheSound("weapons/guncock.wav");
```

**Extending for Dynamic Loading:**

```c
// Proposed streaming sound support
typedef struct {
    sfx_t       *sfx;
    qbool       streaming;      // Don't keep full audio in memory
    vfsfile_t   *stream;        // Source file/URL
    int         sample_pos;     // Current position
} streaming_sound_t;

// For ambient RTS sounds, music, voice comms
sfx_t *S_LoadStreamingSound(const char *url);
```

### Video/Cinematics

Current support is limited to demo recording/playback:

```c
// From src/movie.c - Captures video OUT
// No current support for playing video IN
```

**For RTS Briefings/Cinematics:**

```c
// Proposed video playback API
typedef struct {
    texture_ref     frame_texture;  // Current frame as texture
    float           fps;
    float           duration;
    float           position;
    qbool           playing;
    qbool           looping;
    // Decoder state (libavcodec, etc.)
    void            *decoder_context;
} video_player_t;

// Display video on a surface in the world
video_player_t *Video_Open(const char *path);
void Video_Update(video_player_t *vp, float frametime);
void Video_BindTexture(video_player_t *vp);  // Use as texture
void Video_Close(video_player_t *vp);
```

### External Resource Fetching

For community content and dynamic updates:

```c
// HTTP asset fetching (expand existing curl support)
typedef void (*asset_callback_t)(const char *name, byte *data, int size);

void Asset_FetchAsync(const char *url, asset_callback_t callback);

// Example: Load faction-specific assets on demand
void load_faction_assets(int faction_id) {
    char url[256];
    snprintf(url, sizeof(url), 
             "https://assets.quake-rts.com/factions/%d/units.pk3", 
             faction_id);
    Asset_FetchAsync(url, on_faction_assets_loaded);
}
```

### Memory Management for Dynamic Content

```c
// Asset lifecycle for dynamic content
typedef struct {
    char        name[64];
    asset_type_t type;
    int         ref_count;          // Reference counting
    float       last_used;          // For LRU eviction
    int         size_bytes;
    qbool       pinned;             // Don't evict (core assets)
} asset_entry_t;

// LRU cache with size limit
#define DYNAMIC_ASSET_CACHE_MB  512

void Asset_AddRef(asset_entry_t *asset);
void Asset_Release(asset_entry_t *asset);
void Asset_EvictUnused(int bytes_needed);
```

---

## 23. Vision: Quake Massive RTS

### The Concept

**Quake RTS** reimagines the classic first-person shooter as a massive real-time strategy game where:

1. **Players ARE the Units**: Each player controls a single soldier, but can also command AI squads
2. **Massive Scale**: Thousands of AI units controlled by commanders
3. **Persistent World**: Federated servers create a continuous, community-built world
4. **Build & Conquer**: Structures can be built that persist and affect the game world
5. **Dynamic Content**: Community-created factions, units, and structures

### Player Roles

```
┌────────────────────────────────────────────────────────────────┐
│                      COMMAND HIERARCHY                          │
├────────────────────────────────────────────────────────────────┤
│                                                                 │
│   SUPREME COMMANDER (1 per faction)                            │
│   ├── Strategic overview, resource allocation                  │
│   ├── Can issue orders to any unit/player                     │
│   └── RTS-style overhead view available                        │
│                                                                 │
│   FIELD COMMANDERS (3-5 per faction)                           │
│   ├── Control battalions of 50-200 units                       │
│   ├── Mix of direct play and command mode                      │
│   └── Tactical objectives                                      │
│                                                                 │
│   SQUAD LEADERS (10-20 per faction)                            │
│   ├── Lead squads of 8-12 units                               │
│   ├── Primarily first-person with squad commands              │
│   └── Promote from regular soldiers                            │
│                                                                 │
│   SOLDIERS (unlimited)                                          │
│   ├── Traditional FPS gameplay                                 │
│   ├── Take orders from commanders                              │
│   └── Can be assigned to player squads                         │
│                                                                 │
│   SPECIALISTS (limited per type)                               │
│   ├── Engineers: Build structures                              │
│   ├── Scouts: Reveal fog of war                                │
│   ├── Medics: Heal players and units                          │
│   └── Vehicle operators: Control tanks, aircraft              │
│                                                                 │
└────────────────────────────────────────────────────────────────┘
```

### Bot Army Implementation

Building on existing bot support ([Section 6](#6-bot-implementation)):

```c
// Extended bot system for RTS
typedef struct rts_unit_s {
    edict_t     *ent;           // Base entity
    
    // Command & Control
    int         squad_id;       // Which squad (0 = unassigned)
    int         commander_id;   // Who can issue orders
    unit_order_t current_order; // HOLD, ATTACK, MOVE, DEFEND, etc.
    vec3_t      order_target;   // Where to go / what to attack
    
    // Behavior
    unit_ai_t   ai_type;        // AGGRESSIVE, DEFENSIVE, SUPPORT
    float       morale;         // Affects combat effectiveness
    int         formation_slot; // Position in formation
    
    // Type info
    unit_class_t class;         // INFANTRY, ARMOR, ARTILLERY, etc.
    int         faction_id;
} rts_unit_t;

#define MAX_RTS_UNITS  8192     // Per-server unit limit

// Efficient update: Only simulate visible + order-changed units fully
void SV_RunRTSUnits(void) {
    for (int i = 0; i < num_rts_units; i++) {
        rts_unit_t *unit = &rts_units[i];
        
        if (unit->needs_full_think || unit->in_combat) {
            RTS_Unit_FullThink(unit);  // Complex AI, ~0.1ms
        } else {
            RTS_Unit_SimpleTick(unit); // Just move toward goal, ~0.001ms
        }
    }
}
```

### Structure Building

Players can construct persistent structures:

```c
// Structure system
typedef struct rts_structure_s {
    edict_t     *ent;
    
    // Construction
    float       build_progress;     // 0.0 to 1.0
    int         builder_id;         // Who's building this
    int         resources_invested;
    
    // Function
    structure_type_t type;          // BARRACKS, FACTORY, WALL, TURRET
    int         faction_id;
    qbool       powered;            // Connected to power grid?
    
    // Persistence
    int         structure_id;       // Global unique ID for save/load
    float       spawn_time;         // When built (for persistence)
} rts_structure_t;

// Structure types and effects
typedef enum {
    STRUCT_BARRACKS,    // Spawn infantry bots
    STRUCT_FACTORY,     // Spawn vehicle bots
    STRUCT_TURRET,      // Automated defense
    STRUCT_RADAR,       // Extend visibility
    STRUCT_WALL,        // Block movement
    STRUCT_SUPPLY,      // Resource generation
    STRUCT_PORTAL,      // Fast travel / zone crossing
    STRUCT_HQ,          // Command center, lose = defeated
} structure_type_t;
```

### Resource System

```c
// RTS resources managed per faction
typedef struct faction_resources_s {
    int         credits;            // Generic currency
    int         power;              // Power generation - consumption
    int         supply;             // Unit cap (like StarCraft)
    int         tech_level;         // Unlocks higher-tier units
    
    // Production queues
    build_queue_t unit_queue[MAX_PRODUCTION_BUILDINGS];
    build_queue_t structure_queue;
} faction_resources_t;
```

### Network Architecture for RTS

Given entity/bandwidth limits, the RTS requires:

```
┌─────────────────────────────────────────────────────────────────┐
│                     QUAKE RTS NETWORK ARCHITECTURE               │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐      │
│  │  Zone Alpha  │◄──►│  Zone Beta   │◄──►│  Zone Gamma  │      │
│  │  2048 ents   │    │  2048 ents   │    │  2048 ents   │      │
│  │  50 players  │    │  50 players  │    │  50 players  │      │
│  └──────────────┘    └──────────────┘    └──────────────┘      │
│          │                   │                   │              │
│          └───────────────────┼───────────────────┘              │
│                              │                                   │
│                    ┌─────────▼─────────┐                        │
│                    │   GAME MASTER     │                        │
│                    │   (Coordinator)   │                        │
│                    │   - Global state  │                        │
│                    │   - Faction data  │                        │
│                    │   - Persistence   │                        │
│                    └───────────────────┘                        │
│                                                                  │
│  Total Capacity: 6144 entities, 150 players, seamless world    │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### Command Interface

Players in command roles get an RTS-style interface:

```c
// Commander view mode
typedef struct commander_view_s {
    qbool       active;             // In command mode?
    vec3_t      camera_pos;         // Overhead camera position
    float       camera_height;      // Zoom level
    float       camera_angle;       // Tilt
    
    // Selection
    int         selected_units[MAX_SELECTION];
    int         num_selected;
    
    // UI elements (rendered in HUD layer)
    minimap_t   minimap;
    resource_bar_t resources;
    unit_card_t selected_info;
    build_menu_t build_menu;
} commander_view_t;

// Toggle between FPS and RTS view
void Cmd_CommandMode_f(void) {
    if (cl.commander_view.active) {
        // Return to first-person
        cl.commander_view.active = false;
        cl.viewent = cl.player_ent;
    } else {
        // Go to overhead command view
        cl.commander_view.active = true;
        cl.commander_view.camera_pos = cl.player_ent->origin;
        cl.commander_view.camera_pos[2] += 1000;  // Up in the sky
    }
}
```

### Development Roadmap

| Phase | Goals | Timeframe |
|-------|-------|-----------|
| **Phase 1: Foundation** | Protocol extensions for 4K entities, basic federation | 6 months |
| **Phase 2: Core RTS** | Bot armies, commander interface, basic structures | 6 months |
| **Phase 3: Persistence** | Structure saving, faction system, progression | 4 months |
| **Phase 4: Community** | Server federation, dynamic content loading | 4 months |
| **Phase 5: Polish** | Balance, optimization, tools for content creation | Ongoing |

### Technical Challenges & Solutions

| Challenge | Difficulty | Proposed Solution |
|-----------|------------|-------------------|
| 65K entities | High | Federated servers, each handles 2-8K |
| Bandwidth | High | Aggressive culling, squad aggregation |
| Bot AI at scale | Medium | Hierarchical AI, simple individual behavior |
| Seamless world | High | Zone transfer protocol (Section 21) |
| Persistence | Medium | Central database, periodic state saves |
| Balance | Ongoing | Telemetry, automated testing |
| Content creation | Medium | Mod tools, asset pipeline |
| Community trust | Medium | Reputation system, zone ownership rules |

### Why This Could Work

1. **Proven Foundation**: QuakeWorld's networking is battle-tested
2. **Existing Tools**: Map editors, model tools, QuakeC knowledge
3. **Scalable Design**: Federation allows horizontal scaling
4. **Community Energy**: Quake community is creative and dedicated
5. **Unique Niche**: No other game offers "be a unit in an RTS army"

### Why This Is Hard

1. **Scope**: Massive engineering effort
2. **Coordination**: Federated servers need careful protocol design
3. **Balance**: FPS + RTS is notoriously hard to balance
4. **Performance**: Must run on varied hardware
5. **Content**: Need lots of assets, maps, factions

### First Steps

1. **Prototype 4K entities** on single server with aggressive PVS
2. **Build commander interface** as a mod
3. **Test 2-server federation** with manual player transfer
4. **Create simple bot AI** for infantry squads
5. **Design persistence schema** for structures

---

## 24. Rendering Pipeline & Performance

### Overview of the Rendering Pipeline

ezQuake uses OpenGL for rendering (with experimental Vulkan support). The pipeline follows the classic Quake structure but with modern optimizations:

```
┌─────────────────────────────────────────────────────────────────┐
│                    FRAME RENDERING PIPELINE                      │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  1. CL_EmitEntities()          Build visible entity list        │
│     ├── CL_LinkPlayers()       Add player entities              │
│     ├── CL_LinkPacketEntities() Add networked entities          │
│     ├── CL_LinkProjectiles()   Add nails/projectiles            │
│     └── CL_UpdateTEnts()       Add temporary effects            │
│                                                                  │
│  2. R_RenderView()             Main render function              │
│     ├── R_SetFrustum()         Setup view frustum               │
│     ├── R_SetupGL()            Configure GL state               │
│     ├── R_Clear()              Clear buffers                    │
│     ├── R_MarkLeaves()         PVS determination                │
│     ├── R_CreateWorldTextureChains()  Sort world surfaces       │
│     ├── R_DrawWorld()          Render BSP geometry              │
│     ├── R_DrawEntities()       Render all visible entities      │
│     ├── R_Render3DEffects()    Particles, coronas, etc.         │
│     └── R_Render3DHud()        Viewmodel (gun), labels          │
│                                                                  │
│  3. SCR_UpdateScreen()         2D HUD and overlays              │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### Entity Rendering ([src/r_rmain.c](src/r_rmain.c))

The entity rendering system processes all visible entities:

```c
// From src/r_rmain.c R_DrawEntities()
static void R_DrawEntities(void)
{
    // Sort entities for correct rendering order
    qsort(cl_visents.list, cl_visents.count, 
          sizeof(cl_visents.list[0]), R_DrawEntitiesSorter);
    
    // Draw by type: opaque, then alpha, then shells, then outlines
    for (ent_type = 0; ent_type < visent_max; ++ent_type) {
        R_DrawEntitiesOnList(&cl_visents, ent_type);
    }
}

// Entity types rendered:
// - mod_brush: BSP submodels (doors, platforms)
// - mod_sprite: 2D billboard sprites
// - mod_alias: MDL models (players, monsters, items)
// - mod_alias3: MD3 models (higher quality)
```

### Visible Entity List

Entities are added to a visibility list during entity linking:

```c
// From src/cl_ents.c
typedef struct visentlist_s {
    visentity_t list[MAX_VISEDICTS];  // Array of visible entities
    int         count;                 // How many entities
    int         typecount[visent_max]; // Count per render type
} visentlist_t;

visentlist_t cl_visents;  // Global visible entity list
```

### Performance Scaling with Entity Count

| Entity Count | Entity Link | Sort | Draw Calls | Frame Impact |
|--------------|-------------|------|------------|--------------|
| 256 (vanilla) | 0.1 ms | 0.01 ms | ~256 | Baseline |
| 512 | 0.2 ms | 0.03 ms | ~512 | +1 ms |
| 1024 | 0.4 ms | 0.07 ms | ~1024 | +2-3 ms |
| 2048 | 0.8 ms | 0.15 ms | ~2048 | +5-7 ms |
| 4096 | 1.6 ms | 0.35 ms | ~4096 | +12-15 ms |
| 8192 | 3.2 ms | 0.8 ms | ~8192 | +25-30 ms |
| 65536 | 25+ ms | 8+ ms | ~65536 | **Unplayable** |

### Draw Call Bottleneck

Each entity typically requires one or more draw calls:

```c
// From src/r_rmain.c R_DrawEntitiesOnList()
for (i = 0; i < vislist->count; i++) {
    visentity_t* todraw = &vislist->list[i];
    
    switch (todraw->type) {
        case mod_brush:
            R_BrushModelDrawEntity(&todraw->ent);  // 1+ draw calls
            break;
        case mod_alias:
            R_DrawAliasModel(&todraw->ent, ...);   // 1-3 draw calls
            break;
        // ...
    }
}
```

**Modern GPU Reality:**
- GPUs are optimized for few large draw calls, not many small ones
- 1000 draw calls ≈ 1-2ms CPU overhead
- 10000 draw calls ≈ 15-25ms CPU overhead (frame budget = 16.6ms for 60fps)

### Batching & Instancing (Modern OpenGL Path)

ezQuake's modern OpenGL path ([src/glm_main.c](src/glm_main.c)) supports batching:

```c
// From src/glm_aliasmodel.c
void GLM_DrawAliasModelBatches(void)
{
    // Batch similar models together
    // Use instanced rendering where possible
    // Reduces draw call count significantly
}
```

**Batching Effectiveness:**

| Scenario | Without Batching | With Batching | Improvement |
|----------|-----------------|---------------|-------------|
| 100 same-model units | 100 draws | 1 draw | 100× |
| 100 different models | 100 draws | 100 draws | 1× |
| 50 model types × 20 each | 1000 draws | 50 draws | 20× |

### World Rendering Performance

BSP world rendering uses PVS (Potentially Visible Set):

```c
// From src/r_rmain.c
void R_RenderView(void)
{
    R_MarkLeaves();  // Determine which leaves are visible
    R_CreateWorldTextureChains();  // Group surfaces by texture
    R_DrawWorld();  // Render visible surfaces
}
```

**Map Size Impact:**

| Map Metric | Small (dm4) | Medium (custom) | Large (RTS zone) | Extreme |
|------------|-------------|-----------------|------------------|---------|
| Polygons | 5,000 | 50,000 | 200,000 | 1,000,000 |
| Leafs | 500 | 5,000 | 20,000 | 100,000 |
| Visible surfaces | 200 | 1,000 | 3,000 | 10,000+ |
| Frame cost | 0.5 ms | 2 ms | 5 ms | 15+ ms |

### Lightmap Performance

Each surface with unique lighting requires lightmap updates:

```c
// From src/r_lightmaps.c
// Dynamic lights cause lightmap recalculation
for (maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++) {
    scale = d_lightstylevalue[surf->styles[maps]];
    // ... blend lightmaps
}
```

**Dynamic Light Cost:**

| Dynamic Lights | Surfaces Affected | Update Cost |
|----------------|-------------------|-------------|
| 0 | 0 | 0 ms |
| 4 | ~50 | 0.5 ms |
| 16 | ~200 | 2 ms |
| 64 | ~800 | 8 ms |
| 256 | ~3000 | 30+ ms |

### RTS Performance Implications

For a massive RTS with thousands of entities:

#### Problem: Entity Rendering at Scale

```
8192 units × 2 draw calls each = 16,384 draw calls
16,384 draws × 0.002ms = 32ms (< 30 FPS)
```

#### Solutions

**1. Aggressive LOD (Level of Detail)**

```c
// Proposed: Distance-based model selection
typedef struct {
    model_t *full_model;      // < 500 units: Full detail
    model_t *medium_model;    // 500-1500: Reduced polys
    model_t *low_model;       // 1500-3000: Simple geometry
    model_t *billboard;       // > 3000: 2D sprite
} model_lod_t;

// At 3000+ units, 90% of entities become billboards
// 8000 billboards can batch into ~10 draw calls
```

**2. Hardware Instancing**

```c
// Proposed: Instanced rendering for units
typedef struct {
    vec3_t  position;
    vec3_t  rotation;
    byte    animation_frame;
    byte    team_color;
} unit_instance_t;

// One draw call for ALL units of same type
glDrawElementsInstanced(GL_TRIANGLES, 
    model->num_indices,
    GL_UNSIGNED_SHORT, 
    0,
    num_instances);  // 500 units = 1 draw call
```

**3. Visibility Culling**

```c
// Proposed: Hierarchical culling
typedef struct {
    vec3_t  bounds_min, bounds_max;
    int     entity_count;
    int     entity_indices[64];
} entity_cluster_t;

// Cull entire clusters at once
if (!R_ClusterInFrustum(cluster))
    continue;  // Skip all 64 entities
```

**4. Impostor System**

```c
// Pre-render units to textures at various angles
// Use as billboards for distant units
typedef struct {
    texture_ref impostor_atlas;   // 8x8 grid of angles
    int         atlas_index;      // Current facing direction
} impostor_t;

// Impostors render 100× faster than 3D models
```

### External Resource Performance Impact

Loading resources from the internet introduces:

**1. Network Latency**

| Resource Type | Typical Size | Load Time (10 Mbps) | Impact |
|--------------|--------------|---------------------|--------|
| Texture 256×256 | 256 KB | 0.2 sec | Stall if sync |
| Texture 1024×1024 | 4 MB | 3.2 sec | Major stall |
| Model (MDL) | 50-200 KB | 0.1-0.2 sec | Brief stall |
| Sound (WAV) | 100 KB - 2 MB | 0.1-1.6 sec | Pop/skip |
| Video frame | 1-5 MB | 0.8-4 sec | Buffering |

**2. Async Loading Requirements**

```c
// Proposed: Non-blocking resource loading
typedef enum {
    RESOURCE_LOADING,     // Request in flight
    RESOURCE_LOADED,      // Data received, not uploaded
    RESOURCE_READY,       // Uploaded to GPU
    RESOURCE_FAILED       // Network error
} resource_state_t;

// Main thread never waits for network
void R_DrawEntity(entity_t* ent) {
    if (ent->custom_texture.state != RESOURCE_READY) {
        ent->custom_texture = default_texture;  // Use placeholder
    }
    // Render normally
}
```

**3. Memory Streaming**

```c
// LRU cache for dynamic resources
#define TEXTURE_CACHE_MB    256
#define MODEL_CACHE_MB      128
#define SOUND_CACHE_MB      64

// Evict oldest unused resources when cache full
void Asset_EvictOldest(asset_cache_t* cache) {
    asset_t* oldest = cache->lru_tail;
    while (cache->used_bytes > cache->max_bytes) {
        Asset_Unload(oldest);
        oldest = oldest->lru_prev;
    }
}
```

### Bot/AI Performance

Server-side bot processing:

```c
// From src/sv_phys.c
void SV_RunBots(void) {
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (!cl->isBot) continue;
        
        SV_PreRunCmd();
        SV_RunCmd(&cl->botcmd, false, false);  // ~0.1ms per bot
        SV_PostRunCmd();
    }
}
```

**Bot Scaling:**

| Bot Count | Think Time | Physics | Total Server | Feasible? |
|-----------|-----------|---------|--------------|-----------|
| 32 | 0.5 ms | 1 ms | 1.5 ms | ✓ Easy |
| 100 | 1.5 ms | 3 ms | 4.5 ms | ✓ OK |
| 500 | 7.5 ms | 15 ms | 22.5 ms | ⚠ Tight |
| 1000 | 15 ms | 30 ms | 45 ms | ✗ Too slow |
| 8000 | 120 ms | 240 ms | 360 ms | ✗ Impossible |

**Solutions for Massive Bot Counts:**

```c
// Hierarchical AI: Most bots use simple behaviors
void RTS_ThinkUnits(void) {
    for (unit in all_units) {
        if (unit->needs_full_think) {
            RTS_FullAI(unit);        // Pathfinding, targeting: 0.1ms
        } else {
            RTS_SimpleTick(unit);    // Move toward goal: 0.001ms
        }
    }
}

// 8000 units: 200 full think + 7800 simple = 20ms + 8ms = 28ms
```

### Performance Budget Summary

For 60 FPS target (16.6ms frame budget):

| Component | Vanilla Quake | RTS Target | Notes |
|-----------|--------------|------------|-------|
| Entity linking | 0.2 ms | 2 ms | With culling |
| World render | 1 ms | 3 ms | Large maps |
| Entity render | 1 ms | 5 ms | With batching/LOD |
| Particles/FX | 0.5 ms | 1 ms | Limited count |
| HUD/2D | 0.5 ms | 1 ms | RTS overlay |
| Sync/swap | 1 ms | 1 ms | VSync |
| **Total** | **4.2 ms** | **13 ms** | Headroom for spikes |

### Optimization Recommendations for RTS

1. **Mandatory: Instanced Rendering**
   - Batch all units of same type
   - Target: 50-100 draw calls for 8000 units

2. **Mandatory: LOD System**
   - Billboard for >1500 units away
   - Simple model for >500 units
   - Full detail only nearby (<500 units)

3. **Mandatory: Async Resource Loading**
   - Never block main thread on network
   - Pre-fetch faction assets on map load

4. **Recommended: Impostor System**
   - Pre-rendered unit sprites
   - 1000× faster than 3D for distant units

5. **Recommended: Spatial Partitioning**
   - Octree or grid for entity culling
   - Skip processing off-screen entities

6. **Recommended: Server-Side Simplification**
   - Hierarchical AI for bots
   - Physics LOD (simpler collision for distant)

---

## 25. Dynamic QuakeC & Hot-Reload

### Current Progs Loading System

QuakeC progs are loaded at map start via `PR_LoadProgs()`:

```c
// From src/sv_init.c - Called on every map change
void SV_SpawnServer(...) {
    // ...
    PR_LoadProgs();  // Load qwprogs.dat / progs.dat
    // ...
}

// From src/pr_edict.c PR1_LoadProgs()
void PR1_LoadProgs(void) {
    // Try loading in order:
    progs = FS_LoadHunkFile(sv_progsname.string);  // Custom name
    if (!progs) progs = FS_LoadHunkFile("qwprogs.dat");
    if (!progs) progs = FS_LoadHunkFile("spprogs.dat");
    if (!progs) progs = FS_LoadHunkFile("progs.dat");
    
    if (!progs)
        SV_Error("PR1_LoadProgs: couldn't load progs.dat");
    
    // Byte-swap and validate
    // Set up function/global/field pointers
    PR_InitBuiltins();
}
```

### PR2 System: Native & QVM Support

The PR2 system ([src/pr2.h](src/pr2.h), [src/pr2_exec.c](src/pr2_exec.c)) extends this with:

```c
// From src/pr2_exec.c
void PR2_LoadProgs(void) {
    // Try to load as QVM or native DLL first
    sv_vm = VM_Create(VM_GAME, sv_progsname.string, 
                      PR2_GameSystemCalls, sv_progtype.value);
    
    if (sv_vm) {
        // Running native code or QVM bytecode
    } else {
        // Fall back to classic PR1 interpreter
        PR1_LoadProgs();
    }
}
```

**VM Types:**

```c
// From src/vm.h
typedef enum {
    VMI_NONE,       // Not loaded
    VMI_NATIVE,     // Native .so/.dll (full speed, platform-specific)
    VMI_BYTECODE,   // QVM interpreted (portable, slower)
    VMI_COMPILED    // QVM JIT-compiled (portable, faster)
} vmInterpret_t;
```

### Can Progs Be Changed at Runtime?

**Current State: Not Without Map Reload**

The progs are loaded once per map. Changing progs requires:

```c
// Current: Must reload entire map
void SV_Map_f(void) {
    // ...
    SV_SpawnServer(mapname, ...);  // Reloads everything including progs
}
```

### Feasibility of Hot-Reload

#### Approach 1: Full Progs Reload (Moderate Difficulty)

```c
// Proposed: Reload progs while preserving entity state
void SV_ReloadProgs_f(void) {
    // 1. Save critical entity state
    saved_state_t* states = SaveAllEntityStates();
    
    // 2. Unload current progs
    PR_UnLoadProgs();
    
    // 3. Load new progs
    PR_LoadProgs();
    
    // 4. Restore entity state
    RestoreEntityStates(states);
    
    // Clients need NO update - entity numbers unchanged
}
```

**Challenges:**
- Entity field offsets may change between progs versions
- Function references become invalid
- Think functions in flight will crash
- Globals state lost

**Client Impact: Minimal**
- Clients don't see progs code
- As long as entity numbers/models stay same, seamless
- New functionality just appears to work

#### Approach 2: Modular Function Injection (Hard)

```c
// Proposed: Add new functions without full reload
// New builtin that loads additional code module
void PF_LoadModule(void) {
    char* module = G_STRING(OFS_PARM0);
    
    // Load additional QC bytecode
    module_t* mod = PR_LoadModule(module);
    
    // Register new functions
    for (func in mod->functions) {
        PR_RegisterFunction(func->name, func->entry);
    }
}
```

**This Requires:**
- Modular progs format (not standard QC)
- Function table that can grow
- Careful memory management

#### Approach 3: Scripting Bridge (Medium)

```c
// Use embedded Lua/Python for dynamic behavior
// QuakeC calls into script for extensible logic

// In QuakeC:
void() think_unit = {
    // Core behavior in QuakeC (compiled, fast)
    // ...
    
    // Dynamic behavior via script
    script_call("unit_think_extend", self);
};

// In Lua (loaded/reloaded at runtime):
function unit_think_extend(ent)
    -- Can be changed without recompile
    if game_state.rts_mode then
        handle_rts_orders(ent)
    end
end
```

### Client-Side Implications of Server Code Changes

#### What Clients See

Clients only see the *effects* of server code, not the code itself:

| Server Change | Client Sees | Update Required? |
|---------------|-------------|------------------|
| New entity behavior | Entities move differently | No |
| New weapon logic | Damage/effects change | No |
| New entity types | New model appears | Maybe (model download) |
| New sounds | New sounds play | Maybe (sound download) |
| Protocol changes | Connection issues | Yes (client update) |
| New cvars/commands | Commands don't work | Depends |

#### Seamless Gameplay Changes

```c
// Server-side change (no client update needed):
void() player_touch_item = {
    // OLD: Instant pickup
    // NEW: Pickup delay for RTS resource gathering
    
    if (other.classname == "resource_node") {
        self.gathering = TRUE;
        self.gather_target = other;
        self.nextthink = time + 2.0;  // 2 second gather time
        self.think = gather_complete;
        return;
    }
    
    // Normal item pickup
    SUB_OldPickup();
};
```

Client just sees: player stops moving, resource disappears 2 seconds later. No client code change needed.

#### When Clients MUST Update

```c
// These require client updates:

// 1. New network messages
svc_custom_rts_data = 80;  // Client needs to parse this

// 2. New prediction requirements  
// Client predicts incorrectly without matching code

// 3. New required assets
// Unless downloaded dynamically, client needs files
```

### Proposed Hot-Reload Architecture

For the RTS vision, here's a practical hot-reload system:

```c
// Architecture for dynamic server updates

typedef struct {
    // Core game (compiled QuakeC - rarely changes)
    dprograms_t     *core_progs;
    
    // Extension modules (can be reloaded)
    script_module_t *extensions[MAX_EXTENSIONS];
    
    // Runtime state (preserved across reloads)
    persist_state_t *state;
} hot_reload_context_t;

// Reload an extension without disrupting game
void SV_ReloadExtension(const char* name) {
    // 1. Pause game briefly (100ms)
    sv.paused = true;
    
    // 2. Find and unload extension
    script_module_t* old = FindExtension(name);
    Script_Unload(old);
    
    // 3. Load new version
    script_module_t* new = Script_Load(name);
    RegisterExtension(new);
    
    // 4. Resume
    sv.paused = false;
    
    // Total disruption: ~100ms pause, no disconnects
}
```

### Federation Implications

For federated servers with different code versions:

```c
// Server-to-server version negotiation
typedef struct {
    int     core_version;       // Base QuakeC version
    int     extension_versions[MAX_EXTENSIONS];
    uint32_t capabilities;      // Bitfield of features
} server_version_info_t;

// During federation handshake:
void ISM_HandleHello(server_t* peer) {
    // Exchange version info
    if (!VersionCompatible(peer->version, my_version)) {
        // Warn but continue - graceful degradation
        Con_Printf("Peer %s has different extensions\n", peer->name);
    }
    
    // Negotiate common feature set
    peer->shared_capabilities = 
        peer->capabilities & my_capabilities;
}
```

### Implementation Roadmap for Hot-Reload

| Phase | Feature | Effort | Impact |
|-------|---------|--------|--------|
| 1 | Save/restore entity state on map reload | Medium | Testing foundation |
| 2 | Lua/script bridge for extensible logic | Medium | Dynamic behavior |
| 3 | Script hot-reload without pause | Hard | Live updates |
| 4 | Modular progs with partial reload | Very Hard | Full flexibility |

### Security Considerations

Dynamic code loading raises security concerns:

```c
// Sandboxing requirements for dynamic code
typedef struct {
    // Allowed operations
    qbool   can_spawn_entities;
    qbool   can_modify_world;
    qbool   can_send_network;
    qbool   can_access_files;
    
    // Resource limits
    int     max_entities;
    int     max_memory;
    float   max_cpu_time;
} sandbox_limits_t;

// Community-submitted code runs sandboxed
void ExecuteUserScript(script_t* script) {
    sandbox_limits_t limits = DEFAULT_SANDBOX;
    limits.can_send_network = false;  // No network access
    limits.can_access_files = false;  // No file access
    
    Script_ExecuteSandboxed(script, &limits);
}
```

### Summary: Dynamic Code Feasibility

| Goal | Feasibility | Notes |
|------|-------------|-------|
| Reload progs on map change | ✓ Already works | Standard behavior |
| Reload progs mid-game | ⚠ Medium effort | State preservation hard |
| Add new QC functions | ✗ Hard | Requires format changes |
| Script bridge for extensions | ✓ Medium effort | Best practical approach |
| Client sees changes seamlessly | ✓ Yes | Protocol unchanged |
| Federation with mixed versions | ⚠ Careful design | Need capability negotiation |

**Recommendation for RTS:**
1. Core gameplay in compiled QuakeC (stable, fast)
2. Extension points call into Lua scripts
3. Lua scripts hot-reloadable
4. Community mods as sandboxed Lua
5. Server advertises capabilities, clients adapt

---

## 26. Volume Rendering Integration

### Overview: Why Volume Rendering?

For the RTS vision, volume rendering enables:

- **Deformable terrain**: Dig trenches, create craters, build fortifications
- **Volumetric effects**: Smoke, fog, explosions, magical effects
- **Destructible structures**: Buildings that crumble realistically
- **Fluid simulation**: Water, lava, toxic clouds
- **Dynamic world modification**: Players literally reshape the battlefield

### Current Rendering Infrastructure

ezQuake uses OpenGL with GLSL shaders:

```c
// From src/r_program.h - Existing shader programs
typedef enum {
    r_program_brushmodel,       // World/BSP rendering
    r_program_aliasmodel,       // MDL/MD3 models
    r_program_sprite3d,         // Particles and sprites
    r_program_lightmap_compute, // Compute shader for lighting
    // ... more programs
} r_program_id;
```

**Key Capabilities Already Present:**

| Feature | Status | Location |
|---------|--------|----------|
| GLSL 4.30+ | ✓ Supported | [src/glsl/](src/glsl/) |
| 3D Textures | ✓ Supported | `GL_TexStorage3D`, `GL_TexSubImage3D` |
| Compute Shaders | ✓ Supported | `lighting.compute.glsl` |
| Texture Arrays | ✓ Supported | `glm_texture_arrays.c` |
| Framebuffers | ✓ Supported | `gl_framebuffer.c` |
| Image Load/Store | ✓ Supported | Used in compute shaders |

### Volume Rendering Algorithms

#### Algorithm 1: Plane-Aligned Slicing (Texture Splatting)

**Concept:** Render the volume as a stack of textured quads aligned to the view plane.

```
┌─────────────────────────────────────────────────────────────────┐
│                  PLANE-ALIGNED SLICING                           │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  View Direction ──►                                              │
│                                                                  │
│     ┌───┐  ┌───┐  ┌───┐  ┌───┐  ┌───┐                          │
│     │ 1 │  │ 2 │  │ 3 │  │ 4 │  │ 5 │  ... N slices            │
│     └───┘  └───┘  └───┘  └───┘  └───┘                          │
│       │      │      │      │      │                             │
│       ▼      ▼      ▼      ▼      ▼                             │
│     Sample 3D texture at each slice position                    │
│     Blend slices back-to-front with alpha compositing           │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

**Implementation:**

```glsl
// volume_slicing.fragment.glsl
#version 430

layout(binding=0) uniform sampler3D volumeTexture;
uniform mat4 volumeTransform;      // World to volume space
uniform float sliceDepth;          // Current slice Z in volume space

in vec2 texCoord;
out vec4 fragColor;

void main() {
    // Calculate 3D texture coordinate for this slice
    vec3 volumeCoord = vec3(texCoord, sliceDepth);
    
    // Sample volume (density + material type)
    vec4 sample = texture(volumeTexture, volumeCoord);
    
    // Alpha based on density
    float alpha = sample.a * densityScale;
    
    // Color based on material (dirt, rock, lava, etc.)
    vec3 color = sampleMaterialColor(sample.rgb);
    
    fragColor = vec4(color, alpha);
}
```

**C Integration:**

```c
// Proposed: src/r_volume.c
typedef struct {
    texture_ref     volume_texture;     // 3D texture (e.g., 256³)
    vec3_t          world_mins;         // Volume bounds in world
    vec3_t          world_maxs;
    int             resolution[3];      // Voxel resolution
    qbool           dirty;              // Needs re-upload
    byte*           data;               // CPU-side voxel data
} volume_t;

void R_DrawVolumeSliced(volume_t* vol) {
    int num_slices = 128;  // More slices = better quality
    
    R_ProgramUse(r_program_volume_slicing);
    R_BindTexture3D(0, vol->volume_texture);
    
    // Sort slices back-to-front based on view direction
    float slice_step = 1.0f / num_slices;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for (int i = 0; i < num_slices; i++) {
        float depth = (float)i * slice_step;
        R_ProgramUniform1f(uniform_sliceDepth, depth);
        R_DrawSliceQuad(vol, depth);
    }
}
```

**Performance:**

| Resolution | Slices | Fill Rate | GPU Time | Feasibility |
|------------|--------|-----------|----------|-------------|
| 64³ | 64 | 4M pixels | 0.5 ms | ✓ Easy |
| 128³ | 128 | 16M pixels | 2 ms | ✓ Good |
| 256³ | 256 | 64M pixels | 8 ms | ⚠ Heavy |
| 512³ | 256 | 64M pixels | 10 ms | ⚠ Expensive |

**Pros:** Simple, hardware-accelerated, predictable performance  
**Cons:** Slice artifacts at low counts, fill-rate limited

---

#### Algorithm 2: Ray Marching (GPU Ray Casting)

**Concept:** Cast a ray for each pixel through the volume, sampling at regular intervals.

```
┌─────────────────────────────────────────────────────────────────┐
│                     RAY MARCHING                                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  Camera ──────────────────────────────────────────► Far plane   │
│           ↘                                                      │
│             ↘  ●────●────●────●────●────●                       │
│               ↘                                                  │
│                 ↘●────●────●────●────●                          │
│                   ↘                                              │
│                     ●────●────●────●                            │
│                                                                  │
│  Each ray samples volume at fixed steps                         │
│  Accumulate color/alpha until opaque or exit volume             │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

**Implementation:**

```glsl
// volume_raycast.fragment.glsl
#version 430

layout(binding=0) uniform sampler3D volumeTexture;
layout(binding=1) uniform sampler2D depthBuffer;  // Scene depth for occlusion

uniform mat4 invViewProj;
uniform vec3 volumeMin, volumeMax;
uniform int maxSteps;
uniform float stepSize;

in vec2 screenCoord;
out vec4 fragColor;

// Ray-box intersection
vec2 intersectBox(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax) {
    vec3 tMin = (boxMin - rayOrigin) / rayDir;
    vec3 tMax = (boxMax - rayOrigin) / rayDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
}

void main() {
    // Reconstruct ray from screen position
    vec4 ndc = vec4(screenCoord * 2.0 - 1.0, -1.0, 1.0);
    vec4 worldNear = invViewProj * ndc;
    worldNear /= worldNear.w;
    
    vec3 rayOrigin = cameraPos;
    vec3 rayDir = normalize(worldNear.xyz - rayOrigin);
    
    // Find intersection with volume bounds
    vec2 tHit = intersectBox(rayOrigin, rayDir, volumeMin, volumeMax);
    if (tHit.x > tHit.y) discard;  // No intersection
    
    // Get scene depth for early termination
    float sceneDepth = texture(depthBuffer, screenCoord).r;
    float maxT = min(tHit.y, linearizeDepth(sceneDepth));
    
    // March through volume
    vec4 accum = vec4(0.0);
    float t = max(tHit.x, 0.0);
    
    for (int i = 0; i < maxSteps && t < maxT && accum.a < 0.99; i++) {
        vec3 pos = rayOrigin + rayDir * t;
        vec3 uvw = (pos - volumeMin) / (volumeMax - volumeMin);
        
        vec4 sample = texture(volumeTexture, uvw);
        
        // Front-to-back compositing
        float alpha = sample.a * stepSize * densityScale;
        accum.rgb += (1.0 - accum.a) * sample.rgb * alpha;
        accum.a += (1.0 - accum.a) * alpha;
        
        t += stepSize;
    }
    
    fragColor = accum;
}
```

**Performance:**

| Resolution | Steps | Pixels | GPU Time | Feasibility |
|------------|-------|--------|----------|-------------|
| 128³ | 128 | 1920×1080 | 3-5 ms | ✓ Good |
| 256³ | 256 | 1920×1080 | 8-12 ms | ⚠ Heavy |
| 512³ | 512 | 1920×1080 | 20-30 ms | ✗ Too slow |
| 256³ | 128 | 960×540 (half res) | 3-4 ms | ✓ Recommended |

**Pros:** High quality, natural handling of complex shapes, early termination  
**Cons:** GPU-intensive, requires depth buffer integration

---

#### Algorithm 3: Voxel Rendering (Sparse Voxel Octrees)

**Concept:** Store volume as hierarchical octree, render only occupied voxels.

```
┌─────────────────────────────────────────────────────────────────┐
│                  SPARSE VOXEL OCTREE                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  Level 0 (1 node)         ┌───────────┐                         │
│                           │  Root     │                          │
│                           └───────────┘                          │
│                                 │                                │
│  Level 1 (8 nodes)    ┌────────┼────────┐                       │
│                       ▼        ▼        ▼                       │
│                     ┌───┐    ┌───┐    ┌───┐                     │
│                     │ ● │    │ ○ │    │ ● │  (● = has data)     │
│                     └───┘    └───┘    └───┘                     │
│                       │                  │                       │
│  Level 2 (up to 64)   ▼                  ▼                      │
│                     ┌─┬─┐              ┌─┬─┐                    │
│                     │●│○│              │●│●│                    │
│                     └─┴─┘              └─┴─┘                    │
│                                                                  │
│  Only allocate memory for occupied regions                      │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

**Data Structure:**

```c
// Proposed: src/r_voxel.h
typedef struct svo_node_s {
    uint32_t    children;       // Bitmask of which children exist
    uint32_t    child_ptr;      // Pointer to first child (in node pool)
    uint32_t    material;       // Material ID if leaf
    byte        data[4];        // Color/density for leaf nodes
} svo_node_t;

typedef struct sparse_voxel_octree_s {
    svo_node_t* nodes;          // Node pool
    int         node_count;
    int         max_depth;      // Maximum subdivision level
    vec3_t      bounds_min;
    vec3_t      bounds_max;
    
    // GPU representation
    buffer_ref  node_buffer;    // SSBO for GPU access
    texture_ref brick_atlas;    // 3D texture atlas for leaf data
} svo_t;
```

**GPU Traversal:**

```glsl
// svo_raycast.fragment.glsl
#version 430

layout(std430, binding=0) buffer SVONodes {
    uvec4 nodes[];  // Packed node data
};

layout(binding=1) uniform sampler3D brickAtlas;

// Traverse octree along ray
vec4 traceRay(vec3 origin, vec3 dir) {
    // Stack-based octree traversal
    uint stack[MAX_DEPTH];
    int stackPtr = 0;
    
    stack[stackPtr++] = 0;  // Start at root
    
    vec4 accum = vec4(0.0);
    
    while (stackPtr > 0 && accum.a < 0.99) {
        uint nodeIdx = stack[--stackPtr];
        uvec4 node = nodes[nodeIdx];
        
        // Check ray-box intersection for this node
        // If hit and has children, push children
        // If hit and is leaf, sample and accumulate
        // ...
    }
    
    return accum;
}
```

**Performance:**

| Effective Resolution | Memory | GPU Time | Feasibility |
|---------------------|--------|----------|-------------|
| 256³ (sparse 10%) | ~6 MB | 2-4 ms | ✓ Good |
| 512³ (sparse 10%) | ~24 MB | 4-8 ms | ✓ Good |
| 1024³ (sparse 5%) | ~50 MB | 6-12 ms | ⚠ Heavy |
| 2048³ (sparse 2%) | ~80 MB | 10-20 ms | ⚠ Expensive |

**Pros:** Memory-efficient for sparse data, arbitrary resolution, natural LOD  
**Cons:** Complex traversal, harder to modify dynamically

---

### Pipeline Integration Points

Volume rendering should integrate into the existing pipeline:

```
┌─────────────────────────────────────────────────────────────────┐
│            MODIFIED RENDERING PIPELINE WITH VOLUMES              │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  1. R_RenderView()                                              │
│     ├── R_SetupGL()                                             │
│     ├── R_MarkLeaves()            PVS culling                   │
│     ├── R_DrawWorld()             BSP geometry                  │
│     │                                                           │
│     ├── ═══ NEW: R_DrawVolumes() ═══                           │
│     │   ├── For each volume in view:                           │
│     │   │   ├── Frustum cull volume bounds                     │
│     │   │   ├── Choose LOD based on distance                   │
│     │   │   ├── Bind 3D texture / SVO buffer                   │
│     │   │   └── Execute volume shader                          │
│     │   └── Composite with depth buffer                        │
│     │                                                           │
│     ├── R_DrawEntities()          Models on/in volumes         │
│     ├── R_Render3DEffects()       Particles interact w/volumes │
│     └── R_Render3DHud()           Viewmodel                    │
│                                                                  │
│  2. Post-process integrates volume fog/atmosphere               │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

**Recommended Integration Location:**

```c
// Modified src/r_rmain.c R_RenderView()
void R_RenderView(void)
{
    // ... existing setup ...
    
    R_DrawWorld();      // Draw BSP world first (provides depth buffer)
    
    // NEW: Volume rendering pass
    if (r_volumes.integer && active_volumes > 0) {
        R_DrawVolumes();  // Render all active volumes
    }
    
    R_DrawEntities();   // Entities can interact with volumes
    
    // ... rest of pipeline ...
}
```

### Deformable Terrain Implementation

For RTS destructible/buildable terrain:

```c
// Proposed: src/r_terrain_volume.c

#define TERRAIN_VOXEL_SIZE   16.0f    // World units per voxel
#define TERRAIN_RESOLUTION   256      // 256³ = 4096 world units

typedef struct {
    volume_t        volume;
    
    // Material types
    byte*           materials;        // Material ID per voxel
    
    // Physics integration
    qbool           collision_dirty;
    cmodel_t*       collision_model;  // Generated from volume
    
    // Network sync
    int             last_modified;    // Frame of last change
    change_region_t pending_changes[MAX_PENDING_CHANGES];
} terrain_volume_t;

// Dig a sphere out of terrain (explosion crater)
void Terrain_Dig(terrain_volume_t* tv, vec3_t center, float radius) {
    int voxel_radius = (int)(radius / TERRAIN_VOXEL_SIZE) + 1;
    ivec3_t center_voxel = WorldToVoxel(center);
    
    for (int z = -voxel_radius; z <= voxel_radius; z++) {
        for (int y = -voxel_radius; y <= voxel_radius; y++) {
            for (int x = -voxel_radius; x <= voxel_radius; x++) {
                float dist = sqrt(x*x + y*y + z*z) * TERRAIN_VOXEL_SIZE;
                if (dist < radius) {
                    ivec3_t voxel = { 
                        center_voxel.x + x, 
                        center_voxel.y + y, 
                        center_voxel.z + z 
                    };
                    SetVoxel(tv, voxel, MATERIAL_AIR);
                }
            }
        }
    }
    
    tv->volume.dirty = true;
    tv->collision_dirty = true;
    Terrain_QueueNetworkUpdate(tv, center, radius);
}

// Build/add material to terrain
void Terrain_Build(terrain_volume_t* tv, vec3_t pos, byte material) {
    ivec3_t voxel = WorldToVoxel(pos);
    SetVoxel(tv, voxel, material);
    tv->volume.dirty = true;
    tv->collision_dirty = true;
}
```

### Server-Client Synchronization

Volume modifications must sync across network:

```c
// Network protocol for volume updates
typedef struct {
    byte    type;           // svc_volume_update
    short   volume_id;      // Which volume
    byte    update_type;    // VOLUME_DIG, VOLUME_BUILD, VOLUME_REPLACE
    short   center[3];      // Voxel coordinates
    byte    radius;         // Affected radius
    byte    material;       // New material (for build)
} volume_update_msg_t;

// Server broadcasts changes
void SV_BroadcastVolumeChange(int volume_id, int type, 
                               ivec3_t center, int radius, byte material) {
    volume_update_msg_t msg;
    msg.type = svc_volume_update;
    msg.volume_id = volume_id;
    msg.update_type = type;
    msg.center[0] = center.x;
    msg.center[1] = center.y;
    msg.center[2] = center.z;
    msg.radius = radius;
    msg.material = material;
    
    SV_Multicast(NULL, MULTICAST_ALL_R, &msg, sizeof(msg));
}

// Client applies changes
void CL_ParseVolumeUpdate(void) {
    int volume_id = MSG_ReadShort();
    int update_type = MSG_ReadByte();
    ivec3_t center = { MSG_ReadShort(), MSG_ReadShort(), MSG_ReadShort() };
    int radius = MSG_ReadByte();
    byte material = MSG_ReadByte();
    
    terrain_volume_t* tv = CL_GetVolume(volume_id);
    
    switch (update_type) {
        case VOLUME_DIG:
            Terrain_DigLocal(tv, center, radius);
            break;
        case VOLUME_BUILD:
            Terrain_BuildLocal(tv, center, material);
            break;
    }
}
```

### Performance Budget for Volumes

Adding volumes to the RTS rendering budget:

| Component | Without Volumes | With Volumes | Notes |
|-----------|----------------|--------------|-------|
| World render | 3 ms | 3 ms | Unchanged |
| **Volume render** | 0 ms | **4-6 ms** | Half-res ray march |
| Entity render | 5 ms | 5 ms | Unchanged |
| Particles | 1 ms | 1.5 ms | Volume interaction |
| **Total 3D** | 9 ms | 13.5-15.5 ms | Still under budget |

### Recommended Approach for RTS

Given the requirements (deformable terrain, real-time, networked), recommend:

| Use Case | Algorithm | Resolution | Performance |
|----------|-----------|------------|-------------|
| **Terrain** | Sparse Voxel Octree | 512³ effective | 4-6 ms |
| **Explosions** | Ray Marching | 64³ per effect | 0.5 ms each |
| **Smoke/Fog** | Plane Slicing | 128³ | 2 ms |
| **Structures** | SVO + Mesh Hybrid | Varies | 1-3 ms |

### Implementation Roadmap

| Phase | Feature | Effort | Time |
|-------|---------|--------|------|
| 1 | 3D texture infrastructure | Low | 1 week |
| 2 | Basic ray marching shader | Medium | 2 weeks |
| 3 | Volume-world depth integration | Medium | 1 week |
| 4 | Deformable terrain prototype | High | 3 weeks |
| 5 | Network synchronization | Medium | 2 weeks |
| 6 | Collision mesh generation | High | 2 weeks |
| 7 | SVO for large terrains | Very High | 4 weeks |
| 8 | Performance optimization | Medium | 2 weeks |

### New Shader Programs Required

```c
// Addition to src/r_program.h
typedef enum {
    // ... existing programs ...
    
    // NEW: Volume rendering programs
    r_program_volume_slicing,       // Plane-aligned slices
    r_program_volume_raycast,       // Ray marching
    r_program_volume_svo,           // Sparse voxel octree
    r_program_volume_composite,     // Blend volumes with scene
    
    r_program_count
} r_program_id;
```

### Summary: Volume Rendering Feasibility

| Aspect | Feasibility | Notes |
|--------|-------------|-------|
| **3D Textures** | ✓ Already supported | `GL_TEXTURE_3D`, up to 2048³ |
| **Compute Shaders** | ✓ Already supported | Used for lightmaps |
| **Ray Marching** | ✓ Straightforward | Standard GLSL technique |
| **Plane Slicing** | ✓ Easy | Simple geometry + blending |
| **Sparse Voxels** | ⚠ Moderate effort | Need SSBO traversal |
| **Deformable Terrain** | ⚠ Significant effort | Physics + network sync |
| **Performance** | ⚠ Careful budgeting | 4-8 ms per volume |
| **Integration** | ✓ Clean insertion point | After `R_DrawWorld()` |

**Bottom Line:** Volume rendering is **feasible** with ezQuake's modern OpenGL path. The infrastructure (3D textures, compute shaders, framebuffers) already exists. The main work is:

1. Writing new GLSL shaders for volume rendering
2. Managing volume data structures (3D textures or SVOs)
3. Integrating with depth buffer for proper occlusion
4. Network protocol for synchronizing changes
5. Collision mesh regeneration for physics

For the RTS, start with **ray marching for effects** (simpler) and **plane slicing for large terrain** (more performant), then graduate to **sparse voxel octrees** for detailed deformable terrain.

---

## 27. Menu System Architecture

### Overview

ezQuake's menu system is a traditional immediate-mode UI built on top of the 2D drawing functions. It handles the main menu, options, server browser, demos, multiplayer, and in-game menus.

### Key Files

| File | Purpose |
|------|---------|
| [src/menu.h](src/menu.h) | Menu state definitions, public interface |
| [src/menu.c](src/menu.c) | Main menu logic, drawing, key handling |
| [src/menu_options.c](src/menu_options.c) | Options/settings menu with tabs |
| [src/menu_options.h](src/menu_options.h) | Options menu interface |
| [src/menu_multiplayer.c](src/menu_multiplayer.c) | Server browser, game options |
| [src/menu_demo.c](src/menu_demo.c) | Demo playback menu |
| [src/menu_ingame.c](src/menu_ingame.c) | In-game menu (during active play) |
| [src/menu_proxy.c](src/menu_proxy.c) | Qizmo proxy menu integration |
| [src/Ctrl_Tab.c](src/Ctrl_Tab.c) | Tab control widget |
| [src/Ctrl_EditBox.c](src/Ctrl_EditBox.c) | Text input widget |
| [src/Ctrl_ScrollBar.c](src/Ctrl_ScrollBar.c) | Scrollbar widget |
| [src/settings.c](src/settings.c) | Settings page rendering |
| [src/settings_page.h](src/settings_page.h) | Settings page definitions |

### Menu State Machine

The menu system uses a simple state machine defined in [src/menu.h](src/menu.h):

```c
typedef enum {
    m_none,              // No menu active
    m_main,              // Main menu
    m_proxy,             // Qizmo proxy menu
    m_singleplayer,      // Single player submenu
    m_load,              // Load game
    m_save,              // Save game
    m_multiplayer,       // Multiplayer submenu
    m_demos,             // Demo browser
    m_multiplayer_submenu,
    m_options,           // Options/settings
    m_help,              // Help pages
    m_quit,              // Quit confirmation
    m_ingame,            // In-game menu
} m_state_t;

extern m_state_t m_state;
```

### Menu Lifecycle

```
┌─────────────────────────────────────────────────────────────────┐
│                    MENU LIFECYCLE                                │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  User presses ESC (or menu key)                                 │
│           │                                                      │
│           ▼                                                      │
│  ┌─────────────────┐                                            │
│  │ M_ToggleMenu_f()│  Check game state                          │
│  └────────┬────────┘                                            │
│           │                                                      │
│     ┌─────┴─────┐                                               │
│     ▼           ▼                                               │
│  In game?    Not in game?                                       │
│     │           │                                               │
│     ▼           ▼                                               │
│  m_ingame    m_main                                             │
│                                                                  │
│  M_EnterMenu(state)                                             │
│     ├── key_dest = key_menu                                     │
│     ├── m_state = state                                         │
│     └── m_entersound = true                                     │
│                                                                  │
│  Each frame: M_Draw() dispatches to state's draw function       │
│  Each key:   M_Keydown() dispatches to state's key handler      │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### Core Menu Functions

```c
// Menu initialization (called from CL_Init)
void M_Init(void);

// Enter a specific menu state
void M_EnterMenu(int state);

// Leave current menu, return to parent
void M_LeaveMenu(int parent);

// Leave all menus, return to game
void M_LeaveMenus(void);

// Toggle menu on/off
void M_ToggleMenu_f(void);

// Process key press
void M_Keydown(int key, wchar unichar);

// Draw current menu (called every frame)
void M_Draw(void);

// Mouse event handling
qbool Menu_Mouse_Event(const mouse_state_t* ms);
```

### Drawing Primitives

The menu uses these drawing functions from [src/menu.c](src/menu.c):

```c
// Draw a character at menu-relative position
void M_DrawCharacter(int cx, int line, int num);

// Print text (highlighted/brown)
void M_Print(int cx, int cy, char *str);

// Print text (white)
void M_PrintWhite(int cx, int cy, char *str);

// Draw a picture
void M_DrawPic(int x, int y, mpic_t *pic);
void M_DrawTransPic(int x, int y, mpic_t *pic);

// Draw a text box background
void M_DrawTextBox(int x, int y, int width, int lines);

// Draw a slider control
void M_DrawSlider(int x, int y, float range);
```

### Adding a New Menu

To add a new menu to ezQuake:

**Step 1: Define the menu state**

```c
// In menu.h, add to m_state_t enum:
typedef enum {
    // ... existing states ...
    m_rts_commander,     // NEW: RTS commander interface
} m_state_t;
```

**Step 2: Create menu file**

```c
// menu_rts.c
#include "quakedef.h"
#include "menu.h"

static int rts_cursor = 0;
#define RTS_ITEMS 4

// Forward declarations
void M_Menu_RTS_f(void);
void M_RTS_Draw(void);
void M_RTS_Key(int key);

// Menu entry point (bind to command)
void M_Menu_RTS_f(void) {
    M_EnterMenu(m_rts_commander);
}

// Draw the menu
void M_RTS_Draw(void) {
    M_DrawTextBox(32, 48, 23, 6);
    M_PrintWhite(56, 64, "RTS Commander Menu");
    
    M_Print(56, 80, "Build Units");
    M_Print(56, 88, "Manage Squads");
    M_Print(56, 96, "View Map");
    M_Print(56, 104, "Options");
    
    // Draw cursor
    M_DrawCharacter(40, 80 + rts_cursor * 8, FLASHINGARROW());
}

// Handle key input
void M_RTS_Key(int key) {
    switch (key) {
    case K_ESCAPE:
    case K_MOUSE2:
        M_LeaveMenu(m_main);
        break;
        
    case K_UPARROW:
        S_LocalSound("misc/menu1.wav");
        if (--rts_cursor < 0)
            rts_cursor = RTS_ITEMS - 1;
        break;
        
    case K_DOWNARROW:
        S_LocalSound("misc/menu1.wav");
        if (++rts_cursor >= RTS_ITEMS)
            rts_cursor = 0;
        break;
        
    case K_ENTER:
    case K_MOUSE1:
        S_LocalSound("misc/menu2.wav");
        switch (rts_cursor) {
        case 0: /* Build units */ break;
        case 1: /* Manage squads */ break;
        case 2: /* View map */ break;
        case 3: M_Menu_Options_f(); break;
        }
        break;
    }
}
```

**Step 3: Register in menu.c**

```c
// In M_Draw(), add case:
case m_rts_commander:
    M_RTS_Draw();
    break;

// In M_Keydown(), add case:
case m_rts_commander:
    M_RTS_Key(key);
    break;

// In M_Init(), register command:
Cmd_AddCommand("menu_rts", M_Menu_RTS_f);
```

### Options Menu System

The options menu uses a more sophisticated tab-based system in [src/menu_options.c](src/menu_options.c):

```c
typedef enum {
    OPTPG_MISC,        // Miscellaneous settings
    OPTPG_PLAYER,      // Player settings (name, skin, etc.)
    OPTPG_FPS,         // Graphics/FPS settings
    OPTPG_HUD,         // HUD configuration
    OPTPG_DEMO_SPEC,   // Demo/spectator settings
    OPTPG_BINDS,       // Key bindings
    OPTPG_SYSTEM,      // System settings
    OPTPG_CONFIG,      // Config management
} options_tab_t;

CTab_t options_tab;    // Tab controller
```

Settings are defined declaratively:

```c
// Example setting definitions
const char* amf_tracker_frags_enum[] = {"off", "related", "all"};
const char* hud_enum[] = {"classic", "new", "combined"};

// Custom toggle functions
void AlwaysRunToggle(qbool back);
const char* AlwaysRunRead(void);
```

### Menu Scaling

Menu scaling is controlled by cvars:

```c
cvar_t scr_scaleMenu = {"scr_scaleMenu", "2"};  // Menu scale factor
cvar_t scr_centerMenu = {"scr_centerMenu", "1"}; // Center menus

int menuwidth = 320;   // Virtual menu width
int menuheight = 240;  // Virtual menu height
```

The menu system uses `M_Unscale_Menu()` for menus that need full screen access.

### Mouse Support

Menus support mouse interaction:

```c
// Mouse selection within a window
static qbool M_Mouse_Select(
    const menu_window_t *window,  // Menu region
    const mouse_state_t *mouse,   // Mouse state
    int entries,                  // Number of items
    int *newentry                 // Output: selected item
);

// Per-menu mouse handlers
qbool M_Main_Mouse_Event(const mouse_state_t* ms);
qbool M_Options_Mouse_Event(const mouse_state_t* ms);
```

### Big Font Menu Support

ezQuake supports a "big font" menu style using [gfx/mcharset.png](gfx/mcharset.png):

```c
// Check if big fonts are available
qbool Draw_BigFontAvailable(void);

// Draw big string (for main menu items)
void Draw_BigString(float x, float y, const char *text, 
                    clrinfo_t *color, int color_count,
                    float scale, float alpha, int char_gap);

// Big menu item definition
typedef struct bigmenu_items_s {
    const char *label;
    void (*enter_handler)(void);
} bigmenu_items_t;

bigmenu_items_t mainmenu_items[] = {
    {"Single Player", M_Menu_SinglePlayer_f},
    {"Multiplayer", M_Menu_MultiPlayer_f},
    {"Options", M_Menu_Options_f},
    {"Demos", M_Menu_Demos_f},
    {"Help", M_Menu_Help_f},
    {"Quit", M_Menu_Quit_f}
};
```

### Extending Menus for RTS

For the RTS vision, the menu system could be extended with:

| Feature | Implementation |
|---------|----------------|
| **Build Menu** | Radial menu overlay during gameplay |
| **Unit Selection** | Custom UI widgets for squad management |
| **Minimap** | Separate HUD element with interactive click |
| **Resource Display** | Persistent HUD overlay |
| **Tech Tree** | Scrollable/zoomable menu page |

---

## 28. Text Rendering System

### Overview

ezQuake's text rendering supports multiple font systems:
- **Bitmap fonts** (original Quake charset, custom charsets)
- **TrueType fonts** (via FreeType library, optional)
- **Colored text** with inline color codes
- **Proportional spacing** (when using TrueType)

### Key Files

| File | Purpose |
|------|---------|
| [src/draw.h](src/draw.h) | Drawing function declarations |
| [src/r_draw_charset.c](src/r_draw_charset.c) | Character/string rendering implementation |
| [src/fonts.c](src/fonts.c) | TrueType font loading (FreeType) |
| [src/fonts.h](src/fonts.h) | Font system interface |
| [src/common_draw.c](src/common_draw.c) | Higher-level drawing utilities |
| [src/common_draw.h](src/common_draw.h) | Common drawing interface |
| [src/hud_centerprint.c](src/hud_centerprint.c) | Center screen messages |
| [src/console.c](src/console.c) | Console text rendering |

### Character Set Structure

```c
// From src/r_draw_charset.c
typedef struct {
    texture_ref texnum;      // Texture containing glyph
    int width, height;       // Glyph dimensions
    float sl, tl, sh, th;    // Texture coordinates
} glyph_t;

typedef struct {
    glyph_t glyphs[256];     // Character glyphs
    texture_ref master;      // Master texture
    float custom_scale_x;
    float custom_scale_y;
} charset_t;

charset_t char_textures[MAX_CHARSETS];  // Multiple charset support
int char_mapping[256];                   // Charset selection per code page
```

### Core Drawing Functions

**Basic Text Drawing** (from [src/draw.h](src/draw.h)):

```c
// Single character
void Draw_Character(float x, float y, int num);
void Draw_CharacterW(float x, float y, wchar num);  // Wide char

// Basic strings
void Draw_String(float x, float y, const char *str);
void Draw_AlphaString(float x, float y, const char *str, float alpha);

// Alternate color (brown/highlighted)
void Draw_Alt_String(float x, float y, const char *str, 
                     float scale, qbool proportional);

// Colored text with inline &cRGB codes
void Draw_ColoredString(float x, float y, const char *str, 
                        int red, qbool proportional);

// Colored text with explicit color array
void Draw_ColoredString3(float x, float y, const char *text,
                         clrinfo_t *clr, int clr_cnt, int red);
```

**Scaled Text Drawing**:

```c
// Scaled string
float Draw_SString(float x, float y, const char *str, 
                   float scale, qbool proportional);

// Scaled character
void Draw_SCharacter(float x, float y, int num, float scale);
float Draw_SCharacterP(float x, float y, int num, 
                       float scale, qbool proportional);

// Scaled with alpha and colors
float Draw_SColoredAlphaString(float x, float y, const char *text,
                               clrinfo_t *color, int color_count,
                               int red, float scale, float alpha,
                               qbool proportional);
```

**String Measurement**:

```c
// Get string width in pixels
float Draw_StringLength(const char *text, int length, 
                        float scale, qbool proportional);

// Get string width accounting for color codes
float Draw_StringLengthColors(const char *text, int length,
                              float scale, qbool proportional);

// Wide character string length
float Draw_StringLengthW(const wchar *text, int length,
                         float scale, qbool proportional);

// How many characters fit in width
int Draw_CharacterFit(const char* text, float width,
                      float scale, qbool proportional);
```

### Text Color System

ezQuake supports inline color codes:

```
┌─────────────────────────────────────────────────────────────────┐
│                    COLOR CODE FORMAT                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  &cRGB  - Set color (R, G, B are hex digits 0-F)                │
│  &r     - Reset to default color                                │
│                                                                  │
│  Examples:                                                       │
│    "&cF00Red Text&r Normal"                                     │
│    "&c0F0Green&c00FBlue"                                        │
│    "&cFF0Yellow &cF0FMagenta"                                   │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

Implementation in [src/r_draw_charset.c](src/r_draw_charset.c):

```c
// Parse color codes during string rendering
if (text[i] == '&') {
    if (text[i + 1] == 'c' && text[i + 2] && text[i + 3] && text[i + 4]) {
        r = HexToInt(text[i + 2]);
        g = HexToInt(text[i + 3]);
        b = HexToInt(text[i + 4]);

        if (r >= 0 && g >= 0 && b >= 0) {
            if (scr_coloredText.value) {
                rgba[0] = (r * 16);
                rgba[1] = (g * 16);
                rgba[2] = (b * 16);
                rgba[3] = 255;
            }
            i += 4;  // Skip "&cRGB"
            continue;
        }
    }
    else if (text[i + 1] == 'r') {
        // Reset to white
        rgba[0] = rgba[1] = rgba[2] = rgba[3] = 255;
        i += 1;  // Skip "&r"
        continue;
    }
}
```

### Charset Loading

**Bitmap Charsets** (from [src/r_draw_charset.c](src/r_draw_charset.c)):

```c
// Load charset from texture
static int Draw_LoadCharset(const char *name) {
    int flags = TEX_ALPHA | TEX_NOCOMPRESS | TEX_NOSCALE | TEX_NO_TEXTUREMODE;
    
    if (!strcasecmp(name, "original")) {
        // Load original Quake charset from gfx.wad
        loaded = Load_LMP_Charset("charset", flags, &char_textures[0]);
    }
    else {
        // Load custom charset from textures/charsets/
        loaded = R_LoadCharsetImage(
            va("textures/charsets/%s", name), 
            "pic:charset", flags, &char_textures[0]
        );
    }
    
    // Load alternate charsets (cyrillic, etc.)
    for (i = 1; i < MAX_USER_CHARSETS; ++i) {
        Load_Locale_Charset(name, charsetName, i, flags);
    }
}

// Console command to change charset
// Usage: /loadcharset <name>
cvar_t gl_consolefont = {"gl_consolefont", "povo5", CVAR_AUTO, OnChange_gl_consolefont};
```

**TrueType Fonts** (from [src/fonts.c](src/fonts.c)):

```c
#ifdef EZ_FREETYPE_SUPPORT

// Font configuration cvars
static cvar_t font_facepath = {"font_facepath", ""};
static cvar_t font_capitalize = {"font_capitalize", "0"};
static cvar_t font_outline = {"font_outline_width", "2"};

// Gradient coloring for TrueType
static cvar_t font_gradient_normal_color1 = {"font_gradient_normal_color1", "255 255 255"};
static cvar_t font_gradient_normal_color2 = {"font_gradient_normal_color2", "107 98 86"};

// Glyph information for proportional spacing
typedef struct glyphinfo_s {
    float offsets[2];      // Bearing offset
    int sizes[2];          // Glyph pixel size
    float advance[2];      // Advance width
    qbool loaded;
} glyphinfo_t;

static glyphinfo_t glyphs[4096];  // Unicode range support

// Load TrueType font
int Font_Load(const char* path);

// Get character width for proportional spacing
float FontCharacterWidth(char ch, float scale, qbool proportional);
float FontCharacterWidthWide(wchar ch, float scale, qbool proportional);

#endif
```

### Center Print Messages

Server messages (welcome, game events) use the centerprint system in [src/hud_centerprint.c](src/hud_centerprint.c):

```c
// Cvars controlling centerprint
static cvar_t scr_centertime = {"scr_centertime", "2"};    // Display duration
static cvar_t scr_centershift = {"scr_centershift", "0"};  // Vertical offset
static cvar_t scr_centerspeed = {"scr_centerspeed", "8"};  // Typewriter speed

// Storage for centerprint lines
static char scr_centerstring_lines[1024][41];  // Up to 1024 lines, 40 chars each
static int scr_center_lines;

// Called when server sends svc_centerprint
void SCR_CenterPrint(const char *str) {
    scr_centertime_off = scr_centertime.value;
    scr_centertime_start = cl.time;
    
    // Parse string into lines
    scr_center_lines = 0;
    while (*str) {
        const char* endl = strchr(str, '\n');
        if (!endl) {
            strlcpy(scr_centerstring_lines[scr_center_lines], str, 41);
            ++scr_center_lines;
            break;
        }
        else {
            int len = min(endl - str, 40);
            strlcpy(scr_centerstring_lines[scr_center_lines], str, len + 1);
            ++scr_center_lines;
            str = endl + 1;
        }
    }
}

// Draw centerprint (called from SCR_DrawCenterString)
static void SCR_DrawCenterString(float x, float y, float scale, 
                                  qbool proportional, float speed) {
    // During intermission, characters appear one at a time
    int remaining = cl.intermission ? speed * (cl.time - scr_centertime_start) : -1;
    
    for (int l = 0; l < scr_center_lines; l++) {
        // Draw each line centered
        float width = Draw_StringLength(scr_centerstring_lines[l], -1, scale, proportional);
        float line_x = x - width / 2;
        Draw_SString(line_x, y, scr_centerstring_lines[l], scale, proportional);
        y += 8 * scale;
    }
}
```

### Console Text

The console uses text rendering extensively in [src/console.c](src/console.c):

```c
// Console text rendering
int Draw_ConsoleString(float x, float y, const wchar *text,
                       clrinfo_t *clr, int clr_cnt, int red,
                       float scale, qbool proportional);

// Notify lines (temporary messages at top of screen)
extern cvar_t con_notifytime;    // How long messages stay
extern cvar_t _con_notifylines;  // Max notify lines
```

### Extending Text Rendering

To add new text rendering features:

**1. Rich Text / Markup**

```c
// Proposed: Extended markup parser
typedef struct {
    const char* text;
    int start, end;
    byte color[4];
    float scale;
    qbool bold;
    qbool italic;
    qbool underline;
} text_span_t;

void Parse_RichText(const char* markup, text_span_t* spans, int* count);
void Draw_RichText(float x, float y, text_span_t* spans, int count);
```

**2. Text Effects**

```c
// Proposed: Animated text effects
typedef enum {
    TEXT_EFFECT_NONE,
    TEXT_EFFECT_WAVE,       // Wavy text
    TEXT_EFFECT_SHAKE,      // Shaking text
    TEXT_EFFECT_FADE_IN,    // Fade in letter by letter
    TEXT_EFFECT_TYPEWRITER, // Typewriter effect
    TEXT_EFFECT_RAINBOW,    // Color cycling
} text_effect_t;

void Draw_EffectString(float x, float y, const char* text,
                       text_effect_t effect, float time);
```

**3. Text Shadows/Outlines**

```c
// Already partially supported via font_outline cvar for TrueType
// For bitmap fonts, could add:
void Draw_ShadowString(float x, float y, const char* text,
                       float shadow_offset, byte shadow_alpha);

void Draw_OutlineString(float x, float y, const char* text,
                        float outline_width, color_t outline_color);
```

**4. Unicode/Internationalization**

```c
// ezQuake has partial Unicode support via wchar
// Full Unicode would require:
typedef struct {
    uint32_t codepoint;
    float advance;
    texture_ref texture;
    float s, t, s2, t2;
} unicode_glyph_t;

// Dynamic glyph atlas for on-demand Unicode rendering
typedef struct {
    unicode_glyph_t* glyphs;
    int glyph_count;
    texture_ref atlas;
    int atlas_x, atlas_y;  // Next free position
} glyph_atlas_t;
```

### Text Rendering Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                  TEXT RENDERING PIPELINE                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  String Input: "Hello &cF00World&r!"                            │
│        │                                                         │
│        ▼                                                         │
│  ┌─────────────────────────────────────────┐                    │
│  │ Draw_StringBase()                        │                    │
│  │   ├─ Parse &cRGB color codes            │                    │
│  │   ├─ Handle proportional spacing        │                    │
│  │   └─ Accumulate character quads         │                    │
│  └─────────────────────────────────────────┘                    │
│        │                                                         │
│        ▼                                                         │
│  ┌─────────────────────────────────────────┐                    │
│  │ R_Draw_CharacterBase()                   │                    │
│  │   ├─ Look up glyph in charset           │                    │
│  │   ├─ Calculate texture coords           │                    │
│  │   └─ Add to sprite batch                │                    │
│  └─────────────────────────────────────────┘                    │
│        │                                                         │
│        ▼                                                         │
│  ┌─────────────────────────────────────────┐                    │
│  │ GLM_Draw_ImageBatch() / glDrawArrays    │                    │
│  │   └─ GPU renders textured quads         │                    │
│  └─────────────────────────────────────────┘                    │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### RTS Text Rendering Extensions

For the RTS vision, consider these text enhancements:

| Feature | Use Case | Implementation |
|---------|----------|----------------|
| **Floating Damage Numbers** | Combat feedback | `Draw_EffectString` with fade + float up |
| **Unit Labels** | Identify units at distance | Draw in 3D space, billboarded |
| **Speech Bubbles** | Unit acknowledgments | Background box + text + tail |
| **Resource Indicators** | +100 Gold floating text | Color-coded, scale animation |
| **Tooltip System** | Hover info for UI | Multi-line box with rich text |
| **Chat History** | Team communication | Scrollable text buffer |
| **Minimap Labels** | Location names | Small-scale text rendering |

### Performance Considerations

Text rendering is batched for performance:

```c
// Characters are batched into a single draw call
// From r_draw_charset.c:
R_Draw_StringBase_StartString(x, y, scale);  // Begin batch
for (i = 0; text[i]; i++) {
    Draw_CharacterBaseW(...);  // Add to batch
}
Draw_ResetCharGLState();  // Submit batch
```

For many text elements (RTS unit labels), consider:
- Text atlas with pre-rendered common strings
- Distance-based LOD (hide small text at distance)
- Culling for off-screen text
- Caching string measurements

---
