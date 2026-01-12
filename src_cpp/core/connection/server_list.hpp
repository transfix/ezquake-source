/**
 * @file server_list.hpp
 * @brief Server list management for favorites and recent servers
 * 
 * Manages a list of server addresses with descriptions, supporting
 * load/save operations and dynamic updates.
 * 
 * Original C: cl_slist.c, cl_slist.h
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <functional>
#include "../net/net_types.hpp"

namespace ezquake {

//=============================================================================
// Constants
//=============================================================================

namespace server_list {
    /// Maximum entries in server list (original: MAX_SERVER_LIST)
    constexpr size_t MAX_ENTRIES = 512;
    
    /// Default filename for server list
    constexpr std::string_view DEFAULT_FILENAME = "servers.txt";
    
    /// Separator between server and description in file
    constexpr char FIELD_SEPARATOR = '\t';
}

//=============================================================================
// Server Entry
//=============================================================================

/**
 * @brief A single server entry with address and description.
 * 
 * Original: server_entry_t struct
 */
class ServerEntry {
public:
    ServerEntry() = default;
    
    ServerEntry(std::string_view address, std::string_view description = {})
        : address_(address)
        , description_(description)
    {}
    
    //=========================================================================
    // Accessors
    //=========================================================================
    
    [[nodiscard]] const std::string& address() const noexcept { return address_; }
    [[nodiscard]] const std::string& description() const noexcept { return description_; }
    
    [[nodiscard]] bool hasDescription() const noexcept { 
        return !description_.empty(); 
    }
    
    [[nodiscard]] bool isEmpty() const noexcept {
        return address_.empty();
    }
    
    [[nodiscard]] bool isValid() const noexcept {
        return !address_.empty();
    }
    
    //=========================================================================
    // Mutators
    //=========================================================================
    
    void setAddress(std::string_view addr) {
        address_ = addr;
    }
    
    void setDescription(std::string_view desc) {
        description_ = desc;
    }
    
    void set(std::string_view addr, std::string_view desc = {}) {
        address_ = addr;
        description_ = desc;
    }
    
    void clear() noexcept {
        address_.clear();
        description_.clear();
    }
    
    //=========================================================================
    // Comparison
    //=========================================================================
    
    [[nodiscard]] bool operator==(const ServerEntry& other) const noexcept {
        return address_ == other.address_;
    }
    
    [[nodiscard]] bool operator!=(const ServerEntry& other) const noexcept {
        return !(*this == other);
    }
    
    [[nodiscard]] bool matchesAddress(std::string_view addr) const noexcept {
        return address_ == addr;
    }
    
    //=========================================================================
    // Display
    //=========================================================================
    
    /**
     * @brief Get display string (address or "address - description").
     */
    [[nodiscard]] std::string displayString() const {
        if (description_.empty()) {
            return address_;
        }
        return address_ + " - " + description_;
    }
    
    /**
     * @brief Format for file storage.
     */
    [[nodiscard]] std::string toFileFormat() const {
        if (description_.empty()) {
            return address_;
        }
        return address_ + server_list::FIELD_SEPARATOR + description_;
    }
    
    /**
     * @brief Parse from file format.
     */
    static ServerEntry fromFileFormat(std::string_view line) {
        auto tabPos = line.find(server_list::FIELD_SEPARATOR);
        if (tabPos == std::string_view::npos) {
            return ServerEntry{line};
        }
        return ServerEntry{
            line.substr(0, tabPos),
            line.substr(tabPos + 1)
        };
    }

private:
    std::string address_;
    std::string description_;
};

//=============================================================================
// Server List
//=============================================================================

/**
 * @brief Manages a list of server entries with persistence.
 * 
 * Original: slist[] array and SList_* functions
 */
class ServerList {
public:
    using Entry = ServerEntry;
    using Container = std::vector<Entry>;
    using Iterator = Container::iterator;
    using ConstIterator = Container::const_iterator;
    
    //=========================================================================
    // Construction
    //=========================================================================
    
    ServerList() {
        entries_.reserve(server_list::MAX_ENTRIES);
    }
    
    explicit ServerList(size_t maxEntries)
        : maxEntries_(maxEntries)
    {
        entries_.reserve(maxEntries_);
    }
    
    //=========================================================================
    // Capacity
    //=========================================================================
    
    [[nodiscard]] size_t size() const noexcept { return entries_.size(); }
    [[nodiscard]] size_t capacity() const noexcept { return maxEntries_; }
    [[nodiscard]] bool empty() const noexcept { return entries_.empty(); }
    [[nodiscard]] bool full() const noexcept { return entries_.size() >= maxEntries_; }
    
    //=========================================================================
    // Iteration
    //=========================================================================
    
    [[nodiscard]] Iterator begin() noexcept { return entries_.begin(); }
    [[nodiscard]] Iterator end() noexcept { return entries_.end(); }
    [[nodiscard]] ConstIterator begin() const noexcept { return entries_.begin(); }
    [[nodiscard]] ConstIterator end() const noexcept { return entries_.end(); }
    [[nodiscard]] ConstIterator cbegin() const noexcept { return entries_.cbegin(); }
    [[nodiscard]] ConstIterator cend() const noexcept { return entries_.cend(); }
    
    //=========================================================================
    // Element Access
    //=========================================================================
    
    [[nodiscard]] Entry& operator[](size_t index) { return entries_[index]; }
    [[nodiscard]] const Entry& operator[](size_t index) const { return entries_[index]; }
    
    [[nodiscard]] std::optional<std::reference_wrapper<Entry>> at(size_t index) {
        if (index < entries_.size()) {
            return entries_[index];
        }
        return std::nullopt;
    }
    
    [[nodiscard]] std::optional<std::reference_wrapper<const Entry>> at(size_t index) const {
        if (index < entries_.size()) {
            return entries_[index];
        }
        return std::nullopt;
    }
    
    [[nodiscard]] Entry* get(size_t index) noexcept {
        return (index < entries_.size()) ? &entries_[index] : nullptr;
    }
    
    [[nodiscard]] const Entry* get(size_t index) const noexcept {
        return (index < entries_.size()) ? &entries_[index] : nullptr;
    }
    
    //=========================================================================
    // Lookup
    //=========================================================================
    
    /**
     * @brief Find entry by address.
     * @return Iterator to entry or end() if not found.
     */
    [[nodiscard]] Iterator find(std::string_view address) {
        return std::find_if(entries_.begin(), entries_.end(),
            [address](const Entry& e) { return e.matchesAddress(address); });
    }
    
    [[nodiscard]] ConstIterator find(std::string_view address) const {
        return std::find_if(entries_.begin(), entries_.end(),
            [address](const Entry& e) { return e.matchesAddress(address); });
    }
    
    /**
     * @brief Check if address exists in list.
     */
    [[nodiscard]] bool contains(std::string_view address) const {
        return find(address) != entries_.end();
    }
    
    /**
     * @brief Get index of entry, or -1 if not found.
     */
    [[nodiscard]] int indexOf(std::string_view address) const {
        auto it = find(address);
        if (it == entries_.end()) {
            return -1;
        }
        return static_cast<int>(std::distance(entries_.begin(), it));
    }
    
    /**
     * @brief Find entries matching a predicate.
     */
    template<typename Predicate>
    [[nodiscard]] std::vector<Entry> findIf(Predicate pred) const {
        std::vector<Entry> result;
        std::copy_if(entries_.begin(), entries_.end(), 
            std::back_inserter(result), pred);
        return result;
    }
    
    //=========================================================================
    // Modification
    //=========================================================================
    
    /**
     * @brief Add entry at end.
     * @return true if added, false if list is full.
     */
    bool add(std::string_view address, std::string_view description = {}) {
        if (full()) {
            return false;
        }
        entries_.emplace_back(address, description);
        modified_ = true;
        return true;
    }
    
    /**
     * @brief Add entry at end.
     */
    bool add(const Entry& entry) {
        if (full()) {
            return false;
        }
        entries_.push_back(entry);
        modified_ = true;
        return true;
    }
    
    /**
     * @brief Add or update entry.
     * If address exists, updates description. Otherwise adds new entry.
     */
    bool addOrUpdate(std::string_view address, std::string_view description = {}) {
        auto it = find(address);
        if (it != entries_.end()) {
            it->setDescription(description);
            modified_ = true;
            return true;
        }
        return add(address, description);
    }
    
    /**
     * @brief Set entry at specific index.
     * Original: SList_Set
     */
    bool set(size_t index, std::string_view address, std::string_view description = {}) {
        if (index >= maxEntries_) {
            return false;
        }
        
        // Extend if necessary
        while (entries_.size() <= index) {
            entries_.emplace_back();
        }
        
        entries_[index].set(address, description);
        modified_ = true;
        return true;
    }
    
    /**
     * @brief Insert entry at index, shifting others down.
     */
    bool insert(size_t index, std::string_view address, std::string_view description = {}) {
        if (full()) {
            return false;
        }
        if (index >= entries_.size()) {
            return add(address, description);
        }
        entries_.emplace(entries_.begin() + index, address, description);
        modified_ = true;
        return true;
    }
    
    /**
     * @brief Remove entry by address.
     * @return true if found and removed.
     */
    bool remove(std::string_view address) {
        auto it = find(address);
        if (it == entries_.end()) {
            return false;
        }
        entries_.erase(it);
        modified_ = true;
        return true;
    }
    
    /**
     * @brief Remove entry at index.
     * Original: Similar to SList_Reset behavior
     */
    bool removeAt(size_t index) {
        if (index >= entries_.size()) {
            return false;
        }
        entries_.erase(entries_.begin() + index);
        modified_ = true;
        return true;
    }
    
    /**
     * @brief Reset entry at index (clear but keep position).
     * Original: SList_Reset
     */
    bool resetAt(size_t index) {
        if (index >= entries_.size()) {
            return false;
        }
        entries_[index].clear();
        modified_ = true;
        return true;
    }
    
    /**
     * @brief Swap two entries.
     * Original: SList_Switch
     */
    bool swap(size_t index1, size_t index2) {
        if (index1 >= entries_.size() || index2 >= entries_.size()) {
            return false;
        }
        std::swap(entries_[index1], entries_[index2]);
        modified_ = true;
        return true;
    }
    
    /**
     * @brief Move entry from one position to another.
     */
    bool move(size_t from, size_t to) {
        if (from >= entries_.size() || to >= entries_.size() || from == to) {
            return false;
        }
        
        Entry entry = std::move(entries_[from]);
        entries_.erase(entries_.begin() + from);
        entries_.insert(entries_.begin() + to, std::move(entry));
        modified_ = true;
        return true;
    }
    
    /**
     * @brief Clear all entries.
     */
    void clear() noexcept {
        entries_.clear();
        modified_ = true;
    }
    
    /**
     * @brief Remove empty entries, compacting the list.
     */
    void compact() {
        auto newEnd = std::remove_if(entries_.begin(), entries_.end(),
            [](const Entry& e) { return e.isEmpty(); });
        if (newEnd != entries_.end()) {
            entries_.erase(newEnd, entries_.end());
            modified_ = true;
        }
    }
    
    /**
     * @brief Sort entries by address.
     */
    void sortByAddress() {
        std::sort(entries_.begin(), entries_.end(),
            [](const Entry& a, const Entry& b) { 
                return a.address() < b.address(); 
            });
        modified_ = true;
    }
    
    /**
     * @brief Sort entries by description.
     */
    void sortByDescription() {
        std::sort(entries_.begin(), entries_.end(),
            [](const Entry& a, const Entry& b) { 
                return a.description() < b.description(); 
            });
        modified_ = true;
    }
    
    /**
     * @brief Sort with custom comparator.
     */
    template<typename Compare>
    void sort(Compare comp) {
        std::sort(entries_.begin(), entries_.end(), comp);
        modified_ = true;
    }
    
    //=========================================================================
    // Persistence
    //=========================================================================
    
    /**
     * @brief Load server list from file.
     * Original: SList_Load
     * 
     * File format: one server per line, tab-separated address and description.
     * 
     * @param path File path to load from.
     * @return Number of entries loaded, or -1 on error.
     */
    int load(const std::filesystem::path& path) {
        std::ifstream file(path);
        if (!file) {
            return -1;
        }
        
        clear();
        std::string line;
        int count = 0;
        
        while (std::getline(file, line) && !full()) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#' || line[0] == '/') {
                continue;
            }
            
            // Trim trailing whitespace
            while (!line.empty() && (line.back() == ' ' || line.back() == '\t' || 
                   line.back() == '\r' || line.back() == '\n')) {
                line.pop_back();
            }
            
            if (line.empty()) {
                continue;
            }
            
            entries_.push_back(Entry::fromFileFormat(line));
            ++count;
        }
        
        modified_ = false;
        return count;
    }
    
    /**
     * @brief Save server list to file.
     * Original: SList_Save
     * 
     * @param path File path to save to.
     * @return true if saved successfully.
     */
    bool save(const std::filesystem::path& path) const {
        std::ofstream file(path);
        if (!file) {
            return false;
        }
        
        for (const auto& entry : entries_) {
            if (entry.isValid()) {
                file << entry.toFileFormat() << '\n';
            }
        }
        
        modified_ = false;
        return file.good();
    }
    
    /**
     * @brief Check if list has been modified since last load/save.
     */
    [[nodiscard]] bool isModified() const noexcept { return modified_; }
    
    /**
     * @brief Clear modified flag.
     */
    void clearModified() noexcept { modified_ = false; }
    
    //=========================================================================
    // Statistics
    //=========================================================================
    
    /**
     * @brief Count valid (non-empty) entries.
     * Original: SList_Length
     */
    [[nodiscard]] size_t countValid() const noexcept {
        return std::count_if(entries_.begin(), entries_.end(),
            [](const Entry& e) { return e.isValid(); });
    }
    
    /**
     * @brief Count entries with descriptions.
     */
    [[nodiscard]] size_t countWithDescriptions() const noexcept {
        return std::count_if(entries_.begin(), entries_.end(),
            [](const Entry& e) { return e.hasDescription(); });
    }

private:
    Container entries_;
    size_t maxEntries_ = server_list::MAX_ENTRIES;
    mutable bool modified_ = false;
};

//=============================================================================
// Server Categories
//=============================================================================

/**
 * @brief Categories for server organization.
 */
enum class ServerCategory : uint8_t {
    Favorites,
    Recent,
    Friends,
    Custom
};

/**
 * @brief Named server list with category.
 */
class CategorizedServerList {
public:
    CategorizedServerList(ServerCategory category, std::string_view name)
        : category_(category)
        , name_(name)
    {}
    
    [[nodiscard]] ServerCategory category() const noexcept { return category_; }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] ServerList& list() noexcept { return list_; }
    [[nodiscard]] const ServerList& list() const noexcept { return list_; }
    
    /**
     * @brief Load from standard location.
     */
    int load(const std::filesystem::path& baseDir) {
        auto path = baseDir / (name_ + ".txt");
        return list_.load(path);
    }
    
    /**
     * @brief Save to standard location.
     */
    bool save(const std::filesystem::path& baseDir) const {
        auto path = baseDir / (name_ + ".txt");
        return list_.save(path);
    }

private:
    ServerCategory category_;
    std::string name_;
    ServerList list_;
};

//=============================================================================
// Server List Manager
//=============================================================================

/**
 * @brief Manages multiple server lists (favorites, recent, etc.).
 */
class ServerListManager {
public:
    ServerListManager() {
        // Initialize standard lists
        lists_.emplace_back(ServerCategory::Favorites, "servers");
        lists_.emplace_back(ServerCategory::Recent, "recent");
    }
    
    /**
     * @brief Get the favorites list.
     */
    [[nodiscard]] ServerList& favorites() {
        return lists_[0].list();
    }
    
    [[nodiscard]] const ServerList& favorites() const {
        return lists_[0].list();
    }
    
    /**
     * @brief Get the recent servers list.
     */
    [[nodiscard]] ServerList& recent() {
        return lists_[1].list();
    }
    
    [[nodiscard]] const ServerList& recent() const {
        return lists_[1].list();
    }
    
    /**
     * @brief Get list by category.
     */
    [[nodiscard]] ServerList* getList(ServerCategory category) {
        for (auto& list : lists_) {
            if (list.category() == category) {
                return &list.list();
            }
        }
        return nullptr;
    }
    
    /**
     * @brief Add a custom named list.
     */
    void addCustomList(std::string_view name) {
        lists_.emplace_back(ServerCategory::Custom, name);
    }
    
    /**
     * @brief Load all lists from directory.
     */
    void loadAll(const std::filesystem::path& baseDir) {
        for (auto& list : lists_) {
            list.load(baseDir);
        }
    }
    
    /**
     * @brief Save all modified lists to directory.
     */
    void saveModified(const std::filesystem::path& baseDir) const {
        for (const auto& list : lists_) {
            if (list.list().isModified()) {
                list.save(baseDir);
            }
        }
    }
    
    /**
     * @brief Save all lists to directory.
     */
    void saveAll(const std::filesystem::path& baseDir) const {
        for (const auto& list : lists_) {
            list.save(baseDir);
        }
    }
    
    /**
     * @brief Add server to recent list (at front, remove if exists).
     */
    void addToRecent(std::string_view address, std::string_view description = {}) {
        auto& r = recent();
        
        // Remove if already exists
        r.remove(address);
        
        // Insert at front
        r.insert(0, address, description);
        
        // Keep list at reasonable size
        constexpr size_t MAX_RECENT = 32;
        while (r.size() > MAX_RECENT) {
            r.removeAt(r.size() - 1);
        }
    }
    
    /**
     * @brief Check if server is in favorites.
     */
    [[nodiscard]] bool isFavorite(std::string_view address) const {
        return favorites().contains(address);
    }
    
    /**
     * @brief Toggle favorite status.
     * @return true if now a favorite, false if removed.
     */
    bool toggleFavorite(std::string_view address, std::string_view description = {}) {
        auto& f = favorites();
        if (f.contains(address)) {
            f.remove(address);
            return false;
        } else {
            f.add(address, description);
            return true;
        }
    }

private:
    std::vector<CategorizedServerList> lists_;
};

} // namespace ezquake
