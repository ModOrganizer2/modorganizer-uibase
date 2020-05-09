#include "ifiletree.h"

#include <algorithm>

// FileTreeEntry:
namespace MOBase {
  FileTreeEntry::FileTreeEntry(std::shared_ptr<IFileTree> parent, QString name) :
    m_Parent(parent), m_Name(name), m_Time() {
  }

  FileTreeEntry::FileTreeEntry(std::shared_ptr<IFileTree> parent, QString name, QDateTime time) :
    m_Parent(parent), m_Name(name), m_Time(time) { }

  QString FileTreeEntry::suffix() const {
    const int idx = m_Name.lastIndexOf(".");
    return (isDir() || idx == -1) ? "" : m_Name.mid(idx + 1);
  }

  QString FileTreeEntry::pathFrom(std::shared_ptr<const IFileTree> tree, QString sep) const {

    // We will construct the path from right to left:
    QString path = name();

    auto p = parent();
    while (p != nullptr && p != tree) {
      // We need to check the parent, otherwize we are going to prepend the name
      // and a / for the base, which we do not want.
      if (p->parent() != nullptr) {
        path = p->name() + sep + path;
      }
      p = p->parent();
    }

    return p == tree ? path : QString();
  }

  bool FileTreeEntry::detach() {
    auto p = parent();
    if (p == nullptr) {
      return false;
    }
    return p->erase(shared_from_this()) != p->end();
  }

  bool FileTreeEntry::moveTo(std::shared_ptr<IFileTree> tree) {
    return tree->insert(shared_from_this()) != tree->end();
  }

  std::shared_ptr<FileTreeEntry> FileTreeEntry::createFileEntry(std::shared_ptr<IFileTree> parent, QString name, QDateTime time) {
    return std::shared_ptr<FileTreeEntry>(new FileTreeEntry(parent, name, time));
  }
}

// IFileTree:
namespace MOBase {
  /**
   * Comparator for file entries.
   */
  struct FileEntryComparator {
    bool operator()(std::shared_ptr<FileTreeEntry> const& a, std::shared_ptr<FileTreeEntry> const& b) const {
      if (a->isDir() && !b->isDir()) {
        return true;
      }
      else if (!a->isDir() && b->isDir()) {
        return false;
      }
      else {
        return FileNameComparator::compare(a->name(), b->name()) < 0;
      }
    }
  };


  /**
   *
   */
  bool IFileTree::exists(QString path, FileTypes type) const { return find(path, type) != nullptr; }

  /**
   *
   */
  std::shared_ptr<FileTreeEntry> IFileTree::find(QString path, FileTypes type) {
    return fetchEntry(splitPath(path), type);
  }
  std::shared_ptr<const FileTreeEntry> IFileTree::find(QString path, FileTypes type) const {
    return fetchEntry(splitPath(path), type);
  }

  /**
   *
   */
  std::shared_ptr<FileTreeEntry> IFileTree::addFile(QString path, QDateTime time) {
    QStringList parts = splitPath(path);

    // Check if the file already exists:
    if (fetchEntry(parts, IFileTree::FILE_OR_DIRECTORY) != nullptr) {
      return nullptr;
    }

    // Find or create the tree:
    IFileTree* tree;

    if (parts.size() > 1) {

      // Create the tree:
      std::shared_ptr<FileTreeEntry> treeEntry = createTree(parts.begin(), parts.end() - 1);

      // Early fail if the tree was not created:
      if (treeEntry == nullptr) {
        return nullptr;
      }

      tree = treeEntry->astree().get();
    }
    else {
      tree = this;
    }

    std::shared_ptr<FileTreeEntry> entry = tree->makeFile(this->astree(), parts[parts.size() - 1], time);

    // If makeFile returns a null pointer, it means we cannot create file:
    if (entry == nullptr) {
      return nullptr;
    }

    // Insert in the tree:
    tree->entries().insert(
      std::upper_bound(tree->begin(), tree->end(), entry, FileEntryComparator{}),
      entry
    );

    return entry;
  }

  /**
   *
   */
  std::shared_ptr<IFileTree> IFileTree::addDirectory(QString path) {
    QStringList parts = splitPath(path);
    return createTree(parts.begin(), parts.end());
  }

  /**
   *
   */
  IFileTree::iterator IFileTree::insert(std::shared_ptr<FileTreeEntry> entry, InsertPolicy insertPolicy) {

    // Check that this is not the current tree or a parent tree:
    if (entry->isDir()) {
      std::shared_ptr<IFileTree> tmp = astree();
      while (tmp != nullptr) {
        if (tmp == entry->astree()) {
          return end();
        }
        tmp = tmp->parent();
      }
    }

    // Insert in the tree:
    auto it = std::lower_bound(entries().begin(), entries().end(), entry, FileEntryComparator{});
    bool entryWithSameName = it != entries().end() && (*it)->compare(entry->name()) == 0;

    // Already in the tree?
    if (entryWithSameName && *it == entry) {
      return it;
    }

    // Remove the tree from its parent (parent() can be null
    // if we are inserting a new tree):
    if (entry->parent() != nullptr) {
      entry->parent()->erase(entry);
    }

    if (entryWithSameName) {
      if (insertPolicy == InsertPolicy::FAIL_IF_EXISTS) {
        return end();
      }
      if (insertPolicy == InsertPolicy::REPLACE || entry->isFile()) {
        if (beforeReplace(this, it->get(), entry.get())) {
          *it = entry;
        }
        else {
          return end();
        }
      }
      else {
        mergeTree((*it)->astree(), entry->astree(), nullptr);
      }
    }
    else if (beforeInsert(this, entry.get())) {
      it = entries().insert(it, entry);
    }

    // Update the parent of the tree:
    entry->m_Parent = astree();

    return it;
  }

  /**
   *
   */
  bool IFileTree::move(std::shared_ptr<FileTreeEntry> entry, QString path, InsertPolicy insertPolicy) {

    // Check that this is not a parent tree:
    if (entry->isDir()) {
      std::shared_ptr<IFileTree> tmp = parent();
      while (tmp != nullptr) {
        if (tmp == entry->astree()) {
          return false;
        }
        tmp = tmp->parent();
      }
    }

    // Insert in folder or replace:
    const bool insertFolder = path.isEmpty() || path.endsWith("/") || path.endsWith("\\");

    // Retrieve the path:
    QStringList parts = splitPath(path);

    // Backup the entry name (in case the insertion fails), and update the
    // name:
    QString entryName = entry->m_Name;
    if (!insertFolder) {
      entry->m_Name = parts.takeLast();
    }

    // Find or create the tree:
    IFileTree* tree;

    if (!parts.isEmpty()) {

      // Create the tree:
      std::shared_ptr<FileTreeEntry> treeEntry = createTree(parts.begin(), parts.end());

      // Early fail if the tree was not created:
      if (treeEntry == nullptr) {
        return false;
      }

      tree = treeEntry->astree().get();
    }
    else {
      tree = this;
    }

    // We try to insert, and if it fails we need to reset the name:
    auto it = tree->insert(entry, insertPolicy);
    if (it == tree->end()) {
      entry->m_Name = entryName;
      return false;
    }

    return true;
  }

  /**
   *
   */
  std::size_t IFileTree::merge(std::shared_ptr<IFileTree> source, OverwritesType* overwrites) {

    // Check that this is not a parent tree:
    std::shared_ptr<IFileTree> tmp = astree();
    while (tmp != nullptr) {
      if (tmp == source) {
        return MERGE_FAILED;
      }
      tmp = tmp->parent();
    }

    return mergeTree(astree(), source, overwrites);
  }

  /**
   *
   */
  IFileTree::iterator IFileTree::erase(std::shared_ptr<FileTreeEntry> entry) {

    if (!beforeRemove(this, entry.get())) {
      return end();
    }

    auto it = std::find(begin(), end(), entry);
    if (it == end()) {
      return it;
    }
    entry->m_Parent.reset();
    return entries().erase(it);;
  }

  /**
   *
   */
  std::pair<IFileTree::iterator, std::shared_ptr<FileTreeEntry>> IFileTree::erase(QString name) {
    auto it = std::find_if(begin(), end(), [&name](const auto& entry) {
      return entry->compare(name) == 0;
      });

    if (it == end()) {
      return { it, nullptr };
    }

    if (!beforeRemove(this, it->get())) {
      return { end(), nullptr };
    }

    // Save the entry to return it:
    auto entry = *it;
    entry->m_Parent.reset();

    return { entries().erase(it), entry };
  }

  /**
   *
   */
  bool IFileTree::clear() {
    // Need to find the iterator up to which we should erase:
    auto& entries_ = entries();
    auto it = entries_.begin();
    for (; it != entries_.end() && beforeRemove(this, it->get()); ++it) {
      // Detach (but not remove from the vector):
      (*it)->m_Parent.reset();
    }
    entries_.erase(entries_.begin(), it);
    return empty();
  }

  /**
   *
   */
  std::size_t IFileTree::removeAll(QStringList names) {
    return removeIf([this, &names](auto& entry) {
      return names.contains(entry->name(), Qt::CaseInsensitive); });
  }

  /**
   *
   */
  std::size_t IFileTree::removeIf(std::function<bool(std::shared_ptr<FileTreeEntry> const&)> predicate) {
    std::size_t osize = size();
    auto& en = entries();
    // Cannot use begin() and end() directly because those are immutable iterators:
    en.erase(std::remove_if(en.begin(), en.end(), [this, &predicate](auto& entry) {
      return beforeRemove(this, entry.get()) && predicate(entry);
      }), en.end());
    return osize - size();
  }

  /**
   *
   */
  QStringList IFileTree::splitPath(QString path) {
    // Using raw \\ instead of QDir::separator() since we are replacing by /
    // anyway, and this avoid pulling an extra header (like QDir) only
    // for the separator.
    return path.replace("\\", "/").split("/", QString::SkipEmptyParts);
  }

  /**
   *
   */
  bool IFileTree::beforeReplace(IFileTree const*, FileTreeEntry const*, FileTreeEntry const*) {
    return true;
  }

  /**
   *
   */
  bool IFileTree::beforeInsert(IFileTree const*, FileTreeEntry const*) {
    return true;
  }

  /**
   *
   */
  bool IFileTree::beforeRemove(IFileTree const*, FileTreeEntry const*) {
    return true;
  }

  /**
   * @brief Retrieve the given entry from the given array.
   *
   * @param entries List of entries to search. Must be sorted
   *     according to entry_comparator.
   * @param name Name of the entry to find.
   * @param matchTypes Type of file to check.
   *
   * @return the found entry, or a null pointer if the entry was not found.
   */
  const std::shared_ptr<FileTreeEntry> IFileTree::findEntry(
    std::vector<std::shared_ptr<FileTreeEntry>> const& entries, QString name, FileTypes matchTypes) {
    // Note: Could use the fact that the vector is "sorted". Unfortunately, we don't know what name is
    // (directory or file), so we cannot use FileEntryComparator.
    auto entryIt = std::find_if(
      std::begin(entries), std::end(entries),
      [&](const std::shared_ptr<FileTreeEntry>& fileEntry) {
        return matchTypes.testFlag(fileEntry->fileType()) && fileEntry->compare(name) == 0;
      });

    return entryIt == std::end(entries) ? nullptr : *entryIt;
  }

  /**
   *
   */
  std::size_t IFileTree::mergeTree(
    std::shared_ptr<IFileTree> destination, std::shared_ptr<IFileTree> source, OverwritesType* overwrites) {

    const auto comp = FileEntryComparator{};

    // Number of overwritten entries:
    std::size_t noverwrites = 0;

    // Note: Using the iterators from the vector directly since the other ones
    // cannot be assigned to.
    auto& dstEntries = destination->entries(),
      & srcEntries = source->entries();

    // Note: Since entries are not sorted by name but by type (file/directory)
    // then name, there is no fast way to check for conflict.
    for (auto& srcEntry : srcEntries) {

      // Try to find an exact match (name and type) - This iterator also
      // serve to know where the entry should be inserted:
      auto dstIt = std::lower_bound(
        dstEntries.begin(), dstEntries.end(), srcEntry, comp);

      // Exact match found:
      if (dstIt != dstEntries.end()
        && (*dstIt)->compare(srcEntry->name()) == 0
        && (*dstIt)->isFile() == srcEntry->isFile()) {

        // Both directory, we merge:
        if ((*dstIt)->isDir() && srcEntry->isDir()) {
          noverwrites += mergeTree((*dstIt)->astree(), srcEntry->astree(), overwrites);

          // Detach the entry:
          srcEntry->m_Parent.reset();
        }
        // Otherwize, check if the source can replace the destination:
        else if (beforeReplace(destination.get(), dstIt->get(), srcEntry.get())) {
          // Remove the parent:
          auto dstEntry = *dstIt;
          dstEntry->m_Parent.reset();

          // Update overwrites information:
          noverwrites++;
          if (overwrites != nullptr) {
            overwrites->insert({ dstEntry, srcEntry });
          }

          // Replace the destination:
          *dstIt = srcEntry;
          srcEntry->m_Parent = destination;
        }
        // If not, fails:
        else {
          return MERGE_FAILED;
        }
      }
      else {
        // If we did not find a match, the only way to check is to look
        // through the vector:
        auto conflictIt = std::find_if(dstEntries.begin(), dstEntries.end(), [name = srcEntry->name()](auto const& dstEntry) {
          return dstEntry->compare(name) == 0;
        });

        // Conflict (note that here both entries are of different types, so no need to
        // check if we replace or merge):
        int deleteIndex = -1;
        if (conflictIt != dstEntries.end()) {

          // We check if we can replace the entry:
          if (!beforeReplace(destination.get(), conflictIt->get(), srcEntry.get())) {
            return MERGE_FAILED;
          }

          // We need to store the index because the insert() will mess up the
          // iterators:
          deleteIndex = static_cast<int>(conflictIt - std::begin(dstEntries));
          if (dstIt < conflictIt) {
            deleteIndex += 1;
          }

          // Detach the conflicting entry (we erase it later, after the insertion):
          (*conflictIt)->m_Parent.reset();

          // Update overwrites information:
          noverwrites++;
          if (overwrites != nullptr) {
            overwrites->insert({ *conflictIt, srcEntry });
          }
        }
        // No conflict, we still have to check if we can insert:
        else if (!beforeInsert(destination.get(), srcEntry.get())) {
          return MERGE_FAILED;
        }

        // Insert the entry using the previous iterator:
        dstEntries.insert(dstIt, srcEntry);

        // We delete here:
        if (deleteIndex != -1) {
          dstEntries.erase(dstEntries.begin() + deleteIndex);
        }

        // Update the parent:
        srcEntry->m_Parent = destination;
      }
    }

    // Clear the sources:
    srcEntries.clear();

    return noverwrites;
  }

  /**
   *
   */
  std::shared_ptr<IFileTree> IFileTree::createOrphanTree(QString name) const {
    auto directory = makeDirectory(nullptr, name);
    if (directory != nullptr) {
      directory->m_Populated = true;
    }
    return directory;
  }

  /**
   *
   */
  std::shared_ptr<FileTreeEntry> IFileTree::fetchEntry(QStringList path, FileTypes matchTypes) {
    return std::const_pointer_cast<FileTreeEntry>(const_cast<const IFileTree*>(this)->fetchEntry(path, matchTypes));
  }
  std::shared_ptr<const FileTreeEntry> IFileTree::fetchEntry(QStringList const& path, FileTypes matchTypes) const {
    // Check to ensure that the path contains at least one element:
    if (path.isEmpty()) {
      return nullptr;
    }

    // Early check:
    if (path[path.size() - 1].startsWith("*")) {
      return nullptr;
    }

    const IFileTree* tree = this;
    auto it = std::begin(path);
    for (; tree != nullptr && it != std::end(path) - 1; ++it) {
      // Special cases:
      if (*it == ".") {
        continue;
      }
      else if (*it == "..") {
        tree = tree->parent().get();
      }
      else {
        // Find the entry at the current level:
        std::shared_ptr<FileTreeEntry> entry = findEntry(tree->entries(), *it, IFileTree::DIRECTORY);

        // Early exists if the entry does not exist or is not a directory:
        if (entry == nullptr) {
          tree = nullptr;
        }
        else {
          tree = entry->astree().get();
        }
      }
    }

    if (tree == nullptr) {
      return nullptr;
    }

    // We have the final tree:
    return findEntry(tree->entries(), *it, matchTypes);
  }

  /**
   * @brief Create a new file under this tree.
   *
   * @param parent The current tree, without const-qualification.
   * @param name Name of the file.
   * @param time Modification time of the file.
   *
   * @return the created file.
   */
  std::shared_ptr<FileTreeEntry> IFileTree::makeFile(std::shared_ptr<IFileTree> parent, QString name, QDateTime time) const {
    return createFileEntry(parent, name, time);
  }

  /**
   *
   */
  IFileTree::IFileTree() { }

  /**
   *
   */
  std::shared_ptr<IFileTree> IFileTree::createTree(QStringList::const_iterator begin, QStringList::const_iterator end) {
    // The current tree and entry:
    std::shared_ptr<FileTreeEntry> entry = nullptr;
    std::shared_ptr<IFileTree> tree = astree();
    for (auto it = begin; it != end; ++it) {
      // Special cases:
      if (*it == ".") {
        continue;
      }
      else if (*it == "..") {
        // Must check that we have a parent:
        if (parent() != nullptr) {

        }
        tree = nullptr;
      }
      else {

        // Check if the entry exists (looking for both files and directories
        // because we don't want to override a file):
        entry = findEntry(tree->entries(), *it, IFileTree::FILE_OR_DIRECTORY);

        // Create if it does not:
        if (entry == nullptr) {
          entry = tree->makeDirectory(tree, *it);

          // If makeDirectory returns a null pointer, it means we cannot create tree.
          if (entry == nullptr) {
            break;
          }

          // The tree is empty so already populated:
          entry->astree()->m_Populated = true;

          tree->entries().insert(
            std::upper_bound(tree->begin(), tree->end(), entry, FileEntryComparator{}),
            entry
          );
          tree = entry->astree();
        }
        else if (entry->isDir()) {
          tree = entry->astree();
        }
        else { // Cannot go further:
          tree = nullptr;
        }
      }

      if (tree == nullptr) {
        break;
      }
    }

    return tree;
  }

  /**
   * @brief Retrieve the vector of entries after populating it if required.
   *
   * @return the vector of entries.
   */
  std::vector<std::shared_ptr<FileTreeEntry>>& IFileTree::entries() {
    if (!m_Populated) { populate(); }
    return m_Entries;
  }
  const std::vector<std::shared_ptr<FileTreeEntry>>& IFileTree::entries() const {
    if (!m_Populated) { populate(); }
    return m_Entries;
  }

  /**
   * @brief Populate the internal vectors and update the flag.
   */
  void IFileTree::populate() const {
    doPopulate(std::const_pointer_cast<IFileTree>(astree()), m_Entries);
    m_Populated = true;
    std::sort(std::begin(m_Entries), std::end(m_Entries), FileEntryComparator{});
  }

}