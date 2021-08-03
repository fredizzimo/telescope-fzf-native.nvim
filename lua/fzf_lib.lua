local ffi = require "ffi"

local library_path = (function()
  local dirname = string.sub(debug.getinfo(1).source, 2, #"/fzf_lib.lua" * -1)
  if package.config:sub(1, 1) == "\\" then
    return dirname .. "../build/libfzf.dll"
  else
    return dirname .. "../build/libfzf.so"
  end
end)()
local native = ffi.load(library_path)

ffi.cdef [[
  typedef struct {} fzf_i16_t;
  typedef struct {} fzf_i32_t;
  typedef struct {
    fzf_i16_t I16;
    fzf_i32_t I32;
  } fzf_slab_t;

  typedef struct {} fzf_term_set_t;
  typedef struct {
    fzf_term_set_t **ptr;
    size_t size;
    size_t cap;
  } fzf_pattern_t;
  typedef struct {
    uint32_t *data;
    size_t size;
    size_t cap;
  } fzf_position_t;

  fzf_position_t *fzf_get_positions(const char *text, fzf_pattern_t *pattern, fzf_slab_t *slab);
  void fzf_free_positions(fzf_position_t *pos);
  int32_t fzf_get_score(const char *text, fzf_pattern_t *pattern, fzf_slab_t *slab);

  fzf_pattern_t *fzf_parse_pattern(int32_t case_mode, bool normalize, char *pattern, bool fuzzy);
  void fzf_free_pattern(fzf_pattern_t *pattern);

  fzf_slab_t *fzf_make_default_slab(void);
  void fzf_free_slab(fzf_slab_t *slab);

  // ENTRYMANAGER THINGS
  typedef struct fzf_node_s fzf_node_t;
  struct fzf_node_s {
    fzf_node_t *next;
    fzf_node_t *prev;
    void *item;
  };
  typedef struct {
    fzf_node_t *head;
    fzf_node_t *tail;
    fzf_node_t *_tracked_node;
    size_t len;
    size_t track_at;
  } fzf_linked_list_t;

  fzf_linked_list_t *fzf_list_create(size_t track_at);
  void fzf_list_free(fzf_linked_list_t *list);

  fzf_node_t *fzf_list_append(fzf_linked_list_t *list, void *item);
  void fzf_list_prepend(fzf_linked_list_t *list, void *item);

  void fzf_list_place_after(fzf_linked_list_t *list, size_t index,
                            fzf_node_t *node, void *item);
  void fzf_list_place_before(fzf_linked_list_t *list, size_t index,
                             fzf_node_t *node, void *item);
]]

local fzf = {}

fzf.get_score = function(input, pattern_struct, slab)
  return native.fzf_get_score(input, pattern_struct, slab)
end

fzf.get_pos = function(input, pattern_struct, slab)
  local pos = native.fzf_get_positions(input, pattern_struct, slab)
  local res = {}
  for i = 1, tonumber(pos.size) do
    res[i] = pos.data[i - 1] + 1
  end
  native.fzf_free_positions(pos)

  return res
end

fzf.parse_pattern = function(pattern, case_mode, fuzzy)
  case_mode = case_mode == nil and 0 or case_mode
  fuzzy = fuzzy == nil and true or fuzzy
  local c_str = ffi.new("char[?]", #pattern + 1)
  ffi.copy(c_str, pattern)
  return native.fzf_parse_pattern(case_mode, false, c_str, fuzzy)
end

fzf.free_pattern = function(p)
  native.fzf_free_pattern(p)
end

fzf.allocate_slab = function()
  return native.fzf_make_default_slab()
end

fzf.free_slab = function(s)
  native.fzf_free_slab(s)
end

local LinkedList = {}
LinkedList.__index = LinkedList

function LinkedList:new(opts)
  opts = opts or {}

  local list = ffi.gc(native.fzf_list_create(opts.track_at), native.fzf_list_free)

  return setmetatable({
    list = list,
  }, self)
end

function LinkedList:append(item)
  local node = native.fzf_list_append(self.list, nil)
  node.item = item
end

function LinkedList:prepend(item)
  native.fzf_list_prepend(self.list, item)
end

function LinkedList:size()
  return tonumber(self.list.len)
end

function LinkedList:iter()
  local current_node = self.list.head
  return function()
    local node = current_node
    if node == nil then
      return nil
    end

    current_node = current_node.next
    return node.item
  end
end

function LinkedList:ipairs()
  local index = 0
  local current_node = self.list.head

  return function()
    local node = current_node
    if node == nil then
      return nil
    end

    current_node = current_node.next
    index = index + 1
    return index, node.item, node
  end
end

local EntryManager = {}
EntryManager.__index = EntryManager

function EntryManager:new(max_results, set_entry, info)
  info = info or {}
  info.looped = 0
  info.inserted = 0
  info.find_loop = 0

  -- state contains list of
  --    { entry, score }
  --    Stored directly in a table, accessed as [1], [2]
  set_entry = set_entry or function() end

  return setmetatable({
    linked_states = LinkedList:new { track_at = max_results },
    info = info,
    max_results = max_results,
    set_entry = set_entry,
    worst_acceptable_score = math.huge,
  }, self)
end

function EntryManager:num_results()
  return self.linked_states:size()
end

function EntryManager:get_container(index)
  local count = 0
  for val in self.linked_states:iter() do
    count = count + 1

    if count == index then
      return val
    end
  end

  return {}
end

function EntryManager:get_entry(index)
  return self:get_container(index)[1]
end

function EntryManager:get_score(index)
  return self:get_container(index)[2]
end

function EntryManager:get_ordinal(index)
  return self:get_entry(index).ordinal
end

function EntryManager:find_entry(entry)
  local info = self.info

  local count = 0
  for container in self.linked_states:iter() do
    count = count + 1

    if container[1] == entry then
      info.find_loop = info.find_loop + count

      return count
    end
  end

  info.find_loop = info.find_loop + count
  return nil
end

function EntryManager:_update_score_from_tracked()
  local linked = self.linked_states

  if linked.tracked then
    self.worst_acceptable_score = math.min(self.worst_acceptable_score, linked.tracked[2])
  end
end

function EntryManager:_insert_container_before(picker, index, linked_node, new_container)
  self.linked_states:place_before(index, linked_node, new_container)
  self.set_entry(picker, index, new_container[1], new_container[2], true)

  self:_update_score_from_tracked()
end

function EntryManager:_insert_container_after(picker, index, linked_node, new_container)
  self.linked_states:place_after(index, linked_node, new_container)
  self.set_entry(picker, index, new_container[1], new_container[2], true)

  self:_update_score_from_tracked()
end

function EntryManager:_append_container(picker, new_container, should_update)
  self.linked_states:append(new_container)
  self.worst_acceptable_score = math.min(self.worst_acceptable_score, new_container[2])

  if should_update then
    self.set_entry(picker, self.linked_states:size(), new_container[1], new_container[2])
  end
end

function EntryManager:add_entry(picker, score, entry)
  score = score or 0

  local max_res = self.max_results
  local worst_score = self.worst_acceptable_score
  local size = self.linked_states:size()

  local info = self.info
  info.maxed = info.maxed or 0

  local new_container = { entry, score }

  -- Short circuit for bad scores -- they never need to be displayed.
  --    Just save them and we'll deal with them later.
  if score >= worst_score then
    return self.linked_states:append(new_container)
  end

  -- Short circuit for first entry.
  if size == 0 then
    self.linked_states:prepend(new_container)
    self.set_entry(picker, 1, entry, score)
    return
  end

  for index, container, node in self.linked_states:ipairs() do
    info.looped = info.looped + 1

    if container[2] > score then
      -- print("Inserting: ", picker, index, node, new_container)
      return self:_insert_container_before(picker, index, node, new_container)
    end

    -- Don't add results that are too bad.
    if index >= max_res then
      info.maxed = info.maxed + 1
      return self:_append_container(picker, new_container, false)
    end
  end

  if self.linked_states:size() >= max_res then
    self.worst_acceptable_score = math.min(self.worst_acceptable_score, score)
  end

  return self:_insert_container_after(picker, size + 1, self.linked_states.list.tail, new_container)
end

function EntryManager:iter()
  return coroutine.wrap(function()
    for val in self.linked_states:iter() do
      coroutine.yield(val[1])
    end
  end)
end

fzf.LinkedList = LinkedList
fzf.EntryManager = EntryManager
return fzf
