/*
Mod Organizer shared UI functionality

Copyright (C) 2020 MO2 Team. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef IFILETREE_H
#define IFILETREE_H

#include <map>
#include <memory>
#include <vector>

#include <QFlags>
#include <QDateTime>
#include <QString>

#include "dllimport.h"
#include "utility.h"


/**
 * This header contains definition for the interface IFileTree and the FileTreeEntry
 * class.
 *
 * The purpose of IFileTree is to expose a file tree in a user-friendly way. The
 * IFileTree interface represent a "virtual" file tree: the tree may not exists on
 * the disk or anywhere, it is just an abstract structure. The source of the tree
 * is irrelevant to the IFileTree user and is an implementation details.
 *
 * IFileTree and FileTreeEntry are very intrically linked so it is not possible to
 * use FileTreeEntry for something else, and the only way to create FileTreeEntry
 * is through an existing IFileTree.
 *
 * IFileTree expose a mutable and a strict non-mutable interfaces based on const-qualification
 * of methods, but all underlying implementation are obviously non-const.
 *
 * The IFileTree interface is implemented such that creating implementations should
 * be fairly easy (two short methods to implement). Implementation can override other
 * methods in order to reflect changes from the tree to the actual source (e.g., override
 * the beforeX() methods to actually rename or move file on the disk).
 *
 */

namespace MOBase {

  /**
   *
   */
  class IFileTree;

  /**
   * @brief Simple valid C++ comparator for QString that compare them case-insensitive,
   *     mostly useful to compare filenames on Windows.
   */
  struct FileNameComparator {

    /**
     * @brief The case sensitivity of filenames.
     */
    static constexpr auto CaseSensitivity = Qt::CaseInsensitive;

    /**
     * @brief Compare the two given filenames.
     *
     * @param lhs, rhs Filenames to compare.
     *
     * @return -1, 0 or 1 if the first one is less, equal or greater than the second one.
     */
    static int compare(QString const& lhs, QString const& rhs) {
      return lhs.compare(rhs, Qt::CaseInsensitive);
    }

    /**
     *
     */
    bool operator()(QString const& a, QString const& b) const {
      return compare(a, b) < 0;
    }
  };

  /**
   * @brief Exception thrown when an operation on the tree is not supported by the 
   *     implementation or makes no sense (e.g., creation of a file in an archive).
   */
  struct QDLLEXPORT UnsupportedOperationException: public MyException {
    using MyException::MyException;
  };

  /**
   * @brief Represent an entry in a file tree, either a file or a directory. This class
   *     inherited by IFileTree so that operations on entry are the same for a file or
   *     a directory.
   *
   * This class provides convenience methods to query information on the file, like its
   * name or the its last modification time. It also provides a convenience astree() method
   * that can be used to retrieve the tree corresponding to its entry in case the entry
   * represent a directory.
   *
   */
  class QDLLEXPORT FileTreeEntry : public std::enable_shared_from_this<FileTreeEntry> {

  public: // Enums

    /**
     * @brief Enumeration of the different file type.
     *
     */
    enum FileType {
      DIRECTORY = 0b01, 
      FILE      = 0b10
    };
    Q_DECLARE_FLAGS(FileTypes, FileType);

    constexpr static auto FILE_OR_DIRECTORY = FileTypes{ DIRECTORY, FILE };

  public: // Deleted operators:

    FileTreeEntry(FileTreeEntry const&) = delete;
    FileTreeEntry(FileTreeEntry&&) = delete;

    FileTreeEntry& operator=(FileTreeEntry const&) = delete;
    FileTreeEntry& operator=(FileTreeEntry&&) = delete;

  public: // Methods

    /**
     * @brief Check if this entry is a file.
     *
     * @return true if this entry is a file, false otherwize.
     */
    bool isFile() const {
      return astree() == nullptr;
    }

    /**
     * @brief Check if this entry is a directory.
     *
     * @return true if this entry is a directory, false otherwize.
     */
    bool isDir() const {
      return astree() != nullptr;
    }

    /**
     * @brief Convert this entry to a tree. This method returns a null pointer
     * if this entry corresponds to a file.
     *
     * @return this entry as a tree, or a null pointer if isDir() is false.
     */
    virtual std::shared_ptr<IFileTree> astree() {
      return nullptr;
    }

    /**
     * @brief Convert this entry to a tree. This method returns a null pointer
     * if this entry corresponds to a file.
     *
     * @return this entry as a tree, or a null pointer if isDir() is false.
     */
    virtual std::shared_ptr<const IFileTree> astree() const {
      return nullptr;
    }

    /**
     * @brief Retrieve the type of this entry.
     *
     * @return the type of this entry.
     */
    FileType fileType() const {
      return isDir() ? DIRECTORY : FILE;
    }

    /**
     * @brief Retrieve the name of this entry.
     *
     * @return the name of this entry.
     */
    QString name() const {
      return m_Name;
    }

    /**
     * @brief Compare the name of this entry against the given string.
     *
     * This method only checks the name of the entry, not the full path. 
     *
     * @param name Name to test.
     *
     * @return -1, 0 or 1 depending on the result of the comparison.
     */
    int compare(QString name) const {
      return FileNameComparator::compare(m_Name, name);
    }

    /**
     * @brief Retrieve the full extension of this entry.
     *
     * The full extension is everything after the first dot in
     * the file name.
     *
     * @return the full extension of this entry, or an empty string
     *   if the file has no extension or is directory.
     */
    QString suffix() const;

    /**
     * @brief Retrieve the modification time of this entry.
     *
     * If this entry corresponds to a directory, or if the modification
     * time is unknown, returns an invalid QDateTime.
     *
     * @return the modification time of this file.
     */
    QDateTime time() const {
      return m_Time;
    }

    /**
     * @brief Set the modification time of this entry.
     *
     * @param time New modification time for this entry.
     */
    void setTime(QDateTime time) {
      m_Time = time;
    }

    /**
     * @brief Retrieve the path from this entry up to the root of the tree.
     *
     * This method propagate up the tree so is not constant complexity as
     * the full path is never stored.
     *
     * @param sep The type of separator to use to create the path.
     *
     * @return the path from this entry to the root, including the name
     *     of this entry.
     */
    QString path(QString sep = "\\") const {
      return pathFrom(nullptr, sep);
    }

    /**
     * @brief Retrieve the path from this entry to the given tree.
     *
     * @param tree The tree to reach, must be a parent of this entry.
     * @param sep The type of separator to use to create the path.
     *
     * @return the path from this entry up to the given tree, including the name
     *     of this entry, or QString() if the given tree is not a parent of
     *     this one.
     */
    QString pathFrom(std::shared_ptr<const IFileTree> tree, QString sep = "\\") const;

    /**
     * @brief Detach this entry from its parent tree.
     *
     * @return true if the entry was removed correctly, false otherwize.
     */
    bool detach();

    /**
     * @brief Move this entry to the given tree.
     *
     * @param tree The tree to move this entry to.
     *
     * @return true if the entry was moved correctly, false otherwize.
     */
    bool moveTo(std::shared_ptr<IFileTree> tree);

    /**
     * @brief Retrieve the immediate parent tree of this entry.
     *
     * @return the parent tree containing this entry, or a null pointer
     * if this entry is the root or the parent tree is unreachable.
     */
    std::shared_ptr<IFileTree> parent() {
      return std::const_pointer_cast<IFileTree>(const_cast<const FileTreeEntry*>(this)->parent());
    }

    /**
     * @brief Retrieve the immediate parent tree of this entry.
     *
     * @return the parent tree containing this entry, or a null pointer
     * if this entry is the root or the parent tree is unreachable.
     */
    std::shared_ptr<const IFileTree> parent() const {
      return m_Parent.lock();
    }

  public: // Destructor:

    virtual ~FileTreeEntry() { }

  protected: // Constructors:

    /**
     * @brief Create a new entry corresponding to a file.
     *
     * @param parent The tree containing this file.
     * @param name The name of this file.
     * @param time The modification time of this file.
     */
    FileTreeEntry(std::shared_ptr<IFileTree> parent, QString name, QDateTime time);

    /**
     * @brief Create a new entry corresponding to a directory.
     *
     * @param parent The tree containing this directory.
     * @param name The name of this directory.
     */
    FileTreeEntry(std::shared_ptr<IFileTree> parent, QString name);

    std::weak_ptr<IFileTree> m_Parent;

    QString m_Name;
    QDateTime m_Time;

    friend class IFileTree;
  };

  Q_DECLARE_OPERATORS_FOR_FLAGS(FileTreeEntry::FileTypes);

  /**
   * @brief Interface to classes that provides way to visualize and alter file trees. The tree
   *     may not correspond to an actual file tree on the disk (e.g., inside an archive,
   *     from a QTree Widget, ...).
   *
   * This interface already implements most of the usual methods for a file tree. Child
   * classes only have to implement methods to populate the tree and to create child tree
   * object.
   *
   * In order to prevent wrong usage of the tree, implementing classes may throw 
   * UnsupportedOperationException if an operation is not supported. By default, all operations
   * are supported, but some may not make sense in many situations.
   *
   * The goal of this is not reflect the change made to a IFileTree to the disk, but child
   * classes may override relevant methods to do so.
   *
   * The tree is built upon FileTreeEntry. A given tree holds shared pointers to its entries
   * while each entry holds a weak pointer to its parent, this means that the descending link 
   * are strong (shared pointers) but the uplink are weaks.
   *
   * Accessing the parent is always done by locking the weak pointer so that returned pointer
   * or either null or valid. This structure implies that as long as the initial root lives,
   * entry should not be destroyed, unless the entry are detached from the root and no shared 
   * pointers are kept.
   *
   * However, it is not guarantee that one can go up the tree from a single node entry. If the
   * root node is destroyed, it will not be possible to go up the tree, even if we still have
   * a valid shared pointer.
   *
   * The inheritance is made virtual to provide a way for child classes to use both a custom
   * FileTreeEntry and IFileTree implementations. This has no impact on the usage of the interface.
   *
   */
  class QDLLEXPORT IFileTree : public virtual FileTreeEntry {
  public: // Enumerations and aliases:

    /**
     *
     */
    enum class InsertPolicy {
      FAIL_IF_EXISTS,
      REPLACE, 
      MERGE
    };

    /**
     * @brief Special constant returns by merge when the merge failed.
     */
    constexpr static std::size_t MERGE_FAILED = (std::size_t) -1;

    /**
     *
     */
    using OverwritesType = std::map <std::shared_ptr<const FileTreeEntry>, std::shared_ptr<const FileTreeEntry>> ;

  public: // Iterators:

    /**
     * The standard iterator are constant, but the pointed value are not. Since
     * we are storing a vector of shared pointer to non-const object, we need this
     * wrapper to create iterators to shared pointer of const-object to have proper
     * immutability when IFileTree is const-qualified.
     *
     */
    template <class U, class V>
    struct convert_iterator {

      using reference = U;
      using difference_type = typename V::difference_type;
      using value_type = U;
      using pointer = U;
      using iterator_category = typename V::iterator_category;

      friend bool operator==(convert_iterator a, convert_iterator b) { return a.v != b.v; }
      friend bool operator!=(convert_iterator a, convert_iterator b) { return a.v != b.v; }

      reference operator*() const { return U(*v); }
      reference operator->() const { return U(*v); }

      convert_iterator& operator++() {
        v++;
        return *this;
      }

      value_type operator++(int) {
        value_type value = *(*this);
        (*this)++;
        return *this;
      }

    protected:
      V v;

      convert_iterator(V v) : v{ v } {
      }
      friend class IFileTree;
    };

    using value_type = std::shared_ptr<FileTreeEntry>;
    using reference = std::shared_ptr<FileTreeEntry>;
    using const_reference = std::shared_ptr<const FileTreeEntry>;

    using iterator = std::vector<std::shared_ptr<FileTreeEntry>>::const_iterator;
    using const_iterator = convert_iterator<
      std::shared_ptr<const FileTreeEntry>, 
      std::vector<std::shared_ptr<FileTreeEntry>>::const_iterator>;

    using reverse_iterator = std::vector<std::shared_ptr<FileTreeEntry>>::const_reverse_iterator;
    using const_reverse_iterator = convert_iterator<
      std::shared_ptr<const FileTreeEntry>,
      std::vector<std::shared_ptr<FileTreeEntry>>::const_reverse_iterator>;

  public: // Access methods:

    /**
     *
     */
    iterator begin() { return { std::cbegin(entries()) }; };
    const_iterator begin() const { return { std::cbegin(entries()) }; };
    const_iterator cbegin() const { return { std::cbegin(entries()) }; };

    /**
     *
     */
    reverse_iterator rbegin() { return { std::crbegin(entries()) }; };
    const_reverse_iterator rbegin() const { return { std::crbegin(entries()) }; };
    const_reverse_iterator crbegin() const { return { std::crbegin(entries()) }; };

    /**
     *
     */
    iterator end() { return { std::cend(entries()) }; };
    const_iterator end() const { return { std::cend(entries()) }; };
    const_iterator cend() const { return { std::cend(entries()) }; };

    /**
     *
     */
    reverse_iterator rend() { return { std::crend(entries()) }; };
    const_reverse_iterator rend() const { return { std::crend(entries()) }; };
    const_reverse_iterator crend() const { return { std::crend(entries()) }; };

    /**
     * @brief Retrieve the number of entries in this tree.
     *
     * This is constant if the tree has already been populated.
     *
     * @return the number of entries in this tree.
     */
    std::size_t size() const {
      return entries().size();
    }

    /**
     * @brief Retrieve the file entry at the given index.
     *
     * @param i Index of the entry to retrieve.
     *
     * @return the file entry at the given index, or a null pointer if
     *    the index is invalid.
     */
    std::shared_ptr<FileTreeEntry> at(std::size_t i) {
      if (i < size()) {
        return entries()[i];
      }
      return nullptr;
    }
    std::shared_ptr<const FileTreeEntry> at(std::size_t i) const {
      if (i < size()) {
        return entries()[i];
      }
      return nullptr;
    }

    /**
     * @brief Check if this tree is empty, i.e., if it contains no entries.
     *
     * @return true if the tree is empty, false otherwize.
     */
    bool empty() const {
      return size() == 0;
    }

    /**
     * @brief Check if the given entry exists.
     *
     * @param path Path to the entry, separated by / or \.
     * @param type The type of the entry to check.
     *
     * @return true if the entry was found, false otherwize.
     */
    bool exists(QString path, FileTreeEntry::FileTypes type = FileTreeEntry::FILE_OR_DIRECTORY) const;

    /**
     * @brief Retrieve the given entry.
     *
     * If an entry is found at the given path but does not match the given type,
     * a null pointer is returned.
     *
     * @param path Path to the entry, separated by / or \.
     * @param type The type of the entry to find. 
     *
     * @return the entry if found, a null pointer otherwize.
     */
    std::shared_ptr<FileTreeEntry> find(QString path, FileTypes type = FILE_OR_DIRECTORY);
    std::shared_ptr<const FileTreeEntry> find(QString path, FileTypes type = FILE_OR_DIRECTORY) const;

    /**
     * @brief Convenient method around find() that returns IFileTree instead of entries.
     *
     * @param path Path to the directory, separated by / or \.
     *
     * @return the directory if found, a null pointer otherwize.
     */
    std::shared_ptr<IFileTree> findDirectory(QString path) {
      auto entry = find(path, DIRECTORY);
      return (entry != nullptr && entry->isDir()) ? entry->astree() : nullptr;
    }
    std::shared_ptr<const IFileTree> findDirectory(QString path) const {
      auto entry = find(path, DIRECTORY);
      return (entry != nullptr && entry->isDir()) ? entry->astree() : nullptr;
    }

    /**
     * @brief Retrieve the path from this tree to the given entry.
     *
     * @param entry The entry to reach, must be in this tree.
     * @param sep The type of separator to use to create the path.
     *
     * @return the path from this tree to the given entry, including the name
     *     of the entry, or QString() if the given entry is not in this tree.
     */
    QString pathTo(std::shared_ptr<const FileTreeEntry> entry, QString sep = "\\") const {
      return entry->pathFrom(astree());
    }

  public: // Utility functions:

    /**
     * @brief Create a new orphan empty tree.
     *
     * @param name Name of the tree.
     *
     * @return a new tree without any parent.
     */
    virtual std::shared_ptr<IFileTree> createOrphanTree(QString name = "") const;

  public: // Mutable operations:

    /**
     * @brief Create a new file directly under this tree.
     *
     * This method will return a null pointer if the file already exists. This method
     * invalidates iterators to this tree and all the subtrees present in the
     * given path.
     *
     * @param name Name of the file.
     * @param time Modification time of the file.
     *
     * @return the entry corresponding to the create file, or a null
     *     pointer if the file was not created.
     */
    virtual std::shared_ptr<FileTreeEntry> addFile(QString path, QDateTime time = QDateTime());

    /**
     * @brief Create a new directory tree under this tree.
     *
     * This method will create missing folders in the given path and will
     * not fail if the directory already exists but will fail if the given
     * path contains "." or "..". 
     * This method invalidates iterators to this tree and all the subtrees 
     * present in the given path.
     *
     * @param path Path to the directory.
     *
     * @return the entry corresponding to the created directory, or a null
     *     pointer if the directory was not created.
     */
    virtual std::shared_ptr<IFileTree> addDirectory(QString path);

    /**
     * @brief Insert the given entry in this tree, removing it from its
     * previouis parent.
     *
     * The entry must not be a parent entry of this tree.
     *
     * If the insert policy if FAIL_IF_EXISTS, the call will fail if an entry
     * with the same name already exists. If the policy is REPLACE, an existing
     * entry will be replaced. If MERGE, the entry will be merged with the existing
     * one (if the entry is a file, and a file exists, the file will be replaced).
     *
     * This method invalidates iterator to this tree, to the parent tree of the given
     * entry, and to subtrees of this tree if the insert policy is MERGE.
     *
     * @param entry Entry to insert.
     * @param insertPolicy Policy to use on conflict. 
     *
     * @return an iterator to the inserted tree if it was inserted or if it
     *     already existed, or the end iterator if insertPolicy is FAIL_IF_EXISTS
     *     and an entry with the same name already exists.
     */
    virtual iterator insert(std::shared_ptr<FileTreeEntry> entry, InsertPolicy insertPolicy = InsertPolicy::FAIL_IF_EXISTS);

    /**
     * @brief Merge the given tree with this tree, i.e., insert all entries
     * of the given tree into this tree.
     *
     * The tree must not be a parent entry of this tree. Files present in both tree
     * will be replaced by files in the given tree. The overwrites parameter can
     * be used to track the replaced files. After a merge, the source tree will be
     * empty but still attached to its parent.
     *
     * Note that the merge process makes no distinction between files and directories 
     * when merging: if a directory is present in this tree and a file from source
     * is in conflict with it, the tree will be removed and the file inserted; if a file
     * is in this tree and a directory from source is in conflict with it, the file will
     * be replaced with the directory.
     *
     * This method invalidates iterators to this tree, all the subtrees under this tree 
     * present in the given path, and all the subtrees of the given source.
     *
     * @param source Tree to merge.
     * @param overwrites If not null, can be used to create a mapping from
     *     overriden file to new files.
     *
     * @return the number of overwritten entries, or MERGE_FAILED if the merge
     *     failed (e.g. because the source is a parent of this tree).
     */
    virtual std::size_t merge(
      std::shared_ptr<IFileTree> source, OverwritesType *overwrites = nullptr);

    /**
     * @brief Move the given entry to the given path under this tree.
     *
     * The entry must not be a parent tree of this tree. This method can also be used
     * to rename entries.
     *
     * If the insert policy if FAIL_IF_EXISTS, the call will fail if an entry
     * at the same location already exists. If the policy is REPLACE, an existing
     * entry will be replaced. If MERGE, the entry will be merged with the existing
     * one (if the entry is a file, and a file exists, the file will be replaced).
     *
     * This method invalidates iterator to this tree, to the parent tree of the given
     * entry, and to subtrees of this tree if the insert policy is MERGE.
     *
     * @param entry Entry to insert.
     * @param path The path to move the entry to. If the path ends with / or \,
     *     the entry will be inserted in the corresponding directory instead of replacing
     *     it.
     * @param insertPolicy Policy to use on conflict.
     *
     * @return true if the entry was moved correctly, false otherwize.
     */
    virtual bool move(std::shared_ptr<FileTreeEntry> entry, QString path, InsertPolicy insertPolicy = InsertPolicy::FAIL_IF_EXISTS);

    /**
     * @brief Delete the given entry.
     *
     * @param entry Entry to delete. The entry must belongs to this tree (and
     *     not to a subtree).
     *
     * @return an iterator following the removed entry (might be the end 
     *     iterator if the entry was not found or was the last).
     */
    virtual iterator erase(std::shared_ptr<FileTreeEntry> entry);    
    
    /**
     * @brief Delete the entry with the given name.
     *
     * This method does not recurse into subtrees, so the entry should be 
     * accessible directly from this tree.
     *
     * @param name Name of the entry to delete.
     *
     * @return a pair containing an iterator following the removed entry (might
     *     be the end iterator if the entry was not found or was the last) and
     *     the removed entry (or a null pointer if the entry was not found).
     */
    virtual std::pair<iterator, std::shared_ptr<FileTreeEntry>> erase(QString name);

    /**
     * @brief Delete (detach) all the entries from this tree
     *
     * This method will go through the entries in this tree and stop at the first
     * entry that cannot be deleted, this means that the tree can be partially cleared.
     *
     * @return true if all entries could be deleted, false otherwize.
     */
    virtual bool clear();
    
    /**
     * @brief Delete the entries with the given names from the tree.
     *
     * This method does not recurse into subtrees, so the entry should be
     * accessible directly from this tree. This method invalidates iterators.
     *
     * @param names Names of the entries to delete.
     *
     * @return the number of deleted entry.
     */
    virtual std::size_t removeAll(QStringList names);

    /**
     * @brief Delete the entries that match the given predicate from the tree.
     *
     * This method does not recurse into subtrees, so the entry should be
     * accessible directly from this tree. This method invalidates iterators.
     *
     * @param predicate Predicate that should return true for entries to delete.
     *
     * @return the number of deleted entry.
     */
    virtual std::size_t removeIf(std::function<bool(std::shared_ptr<FileTreeEntry> const& entry)> predicate);

  public: // Inherited methods:

    /**
     * @brief Retrieve the tree corresponding to this entry. Returns a null pointer
     * if this entry corresponds to a file.
     *
     * @return the tree corresponding to this entry, or a null pointer if
     *     isDir() is false.
     */
    virtual std::shared_ptr<IFileTree> astree() override {
      return std::dynamic_pointer_cast<IFileTree>(shared_from_this());
    }

    /**
     * @brief Retrieve the tree corresponding to this entry. Returns a null pointer
     * if this entry corresponds to a file.
     *
     * @return the tree corresponding to this entry, or a null pointer if
     *     isDir() is false.
     */
    virtual std::shared_ptr<const IFileTree> astree() const override {
      return std::dynamic_pointer_cast<const IFileTree>(shared_from_this());
    }

  protected: // Destructor:

    virtual ~IFileTree() {
    }

  public: // Deleted operators:

    IFileTree(IFileTree const&) = delete;
    IFileTree(IFileTree&&) = delete;

    IFileTree& operator=(IFileTree const&) = delete;
    IFileTree& operator=(IFileTree&&) = delete;

  /**
   * A few implementation details here for implementing classes. While there are multiple
   * virtual public methods, most implementation should not have to re-implement them.
   *
   * There are two pure virtual methods that needs to be implemented by any child class:
   *   - makeDirectory(): used to create directory - this is were the creation of the
   *       directory should be reflected to the underlying source if needed (e.g., actual
   *       creation of the directory on the disk), but note that this method is also 
   *       called for existing directories, to obtain corresponding C++ object, so it should
   *       not fail if there directory exists;
   *   - doPopulate(): called when a tree have to be populated.
   *
   * The other commons methods that can be re-implemented are:
   *   - makeFile(), this is used to create new file - this is very similar to makeDirectory()
   *       except that it has a default implementation that simply creates a FileTreeEntry.
   *   - beforeInsert(), beforeReplace() and beforeRemove(): these can be implemented to 1) prevent 
   *       some operations, 2) perform operations on the actual tree (e.g., move a file on the disk).
   *
   * The addFile() and addDirectory() methods can also be re-implemented to prevent operations
   * by throw UnsupportedOperationException if needed.
   */
  protected:

    friend class FileTreeEntry;

    /**
     * Split the given path into parts.
     *
     * @param path The path to split.
     *
     * @return a list containing the section of the path.
     */
    static QStringList splitPath(QString path);

    /**
     * @brief Called before replacing an entry with another one.
     *
     * This is a pre-method meaning that it can be used to prevent an operation on
     * the tree.
     *
     * This method is for internal usage only and is called when an entry is going to
     * be replaced by another (because there is name conflict). 
     *
     * The base implementation of this method does nothing (the actual replacement is
     * made elsewhere). This method can be used to prevent a replacement by returning
     * false.
     *
     * @param dstTree Tree containing the destination entry.
     * @param destination Entry that will be replaced.
     * @param source Entry that will replace the destination.
     *
     * @return true if the entry can be replaced, false otherwize.
     */
    virtual bool beforeReplace(
      IFileTree const* dstTree, FileTreeEntry const* destination, FileTreeEntry const* source);

    /**
     * @brief Called before inserting an entry in a tree.
     *
     * This is a pre-method meaning that it can be used to prevent an operation on
     * the tree.
     *
     * This method is for internal usage only and is called when an entry is going to
     * be inserted in a tree. This method is not called after makeFile() or makeDirectory()
     * so those should be used to prevent creation of files and directories.
     *
     * The base implementation of this method does nothing (the actual insertion is
     * made elsewhere). This method can be used to prevent an insertion by returning
     * false.
     *
     * @param tree Tree into which the entry will be inserted.
     * @param source Entry that will be inserted the destination.
     *
     * @return true if the entry can be inserted, false otherwize.
     */
    virtual bool beforeInsert(IFileTree const* entry, FileTreeEntry const* name);

    /**
     * @brief Called before removing an entry.
     *
     * This is a pre-method meaning that it can be used to prevent an operation on
     * the tree.
     *
     * This method is for internal usage only and is called when an entry is going to
     * be removed from a tree.
     *
     * The base implementation of this method does nothing (the actual removal is
     * made elsewhere). This method can be used to prevent it by returning false.
     *
     * @param tree Tree containing the entry.
     * @param entry Entry that will be removed.
     *
     * @return true if the entry can be removed, false otherwize.
     */
    virtual bool beforeRemove(IFileTree const* entry, FileTreeEntry const* name);

    /**
     * @brief Create a new file under this tree.
     *
     * @param parent The current tree, without const-qualification.
     * @param name Name of the file.
     * @param time Modification time of the file.
     *
     * @return the created file.
     */
    virtual std::shared_ptr<FileTreeEntry> makeFile(std::shared_ptr<IFileTree> parent, QString name, QDateTime time) const;

    /**
     * @brief Create a new entry corresponding to a subtree under this tree.
     *
     * @param parent The current tree, without const-qualification.
     * @param name Name of the directory.
     *
     * @return the entry for the created directory.
     */
    virtual std::shared_ptr<IFileTree> makeDirectory(std::shared_ptr<IFileTree> parent, QString name) const = 0;

    /**
     * @brief Method that subclass should implement.
     *
     * This method should populate the given entries of this tree by using the makeFile and makeTree method. The
     * usage of the makeFile and makeTree method is not mandatory here.
     *
     * @param parent The current tree, without const-qualification.
     * @param entries Vector of entries to populate.
     */
    virtual void doPopulate(std::shared_ptr<IFileTree> parent, std::vector<std::shared_ptr<FileTreeEntry>>& entries) const = 0;

  protected: // Constructor

    /**
     * @brief Creates a new tree. This method takes no parameter since, due to the virtual inheritance,
     *     child classes must directly call the FileTreeEntry constructor.
     */
    IFileTree();

  /**
   *
   */
  private: // Private stuff, look away!

    /**
     * @brief Retrieve the entry corresponding to the given path.
     *
     * @param path Path to entry.
     * @param matchType Type of file to check.
     *
     * @return the entry, or a null pointer if the entry did not exist.
     */
    std::shared_ptr<FileTreeEntry> fetchEntry(QStringList path, FileTypes matchType);
    std::shared_ptr<const FileTreeEntry> fetchEntry(QStringList const& path, FileTypes matchType) const;

    /**
     * @brief Merge the source tree into the destination tree. On conflict, the source entries are always chosen.
     *
     * @param destination Destination tree.
     * @param source Source tree.
     *
     * @return the number of overwritten entries.
     */
    std::size_t mergeTree(
      std::shared_ptr<IFileTree> destination, std::shared_ptr<IFileTree> source, OverwritesType* overwrites);

    /**
     * @brief Create a new subtree under the given tree.
     *
     * This method will create missing folders in the given path and will not fail if the directory 
     * already exists but will fail the given path contains "." or "..".
     *
     * @param begin, end Range of section of the path.
     *
     * @return the entry corresponding to the create tree, or a null pointer if the tree was not created.
     */
    std::shared_ptr<IFileTree> createTree(QStringList::const_iterator begin, QStringList::const_iterator end);
    
    /**
   * @brief Retrieve the given entry from the given array.
   *
   * @param entries List of entries to search. Must be sorted according to entry_comparator.
   * @param name Name of the entry to find.
   * @param matchTypes Type of file to check.
   *
   * @return the found entry, or a null pointer if the entry was not found.
   */
    static const std::shared_ptr<FileTreeEntry> findEntry(
      std::vector<std::shared_ptr<FileTreeEntry>> const& entries, QString name, FileTypes matchTypes);

    // Indicate if this tree has been populated:
    mutable bool m_Populated = false;
    mutable std::vector<std::shared_ptr<FileTreeEntry>> m_Entries;

    /**
     * @brief Retrieve the vector of entries after populating it if required.
     *
     * @return the vector of entries.
     */
    std::vector<std::shared_ptr<FileTreeEntry>>& entries();
    const std::vector<std::shared_ptr<FileTreeEntry>>& entries() const;

    /**
     * @brief Populate the internal vectors and update the flag.
     */
    void populate() const;
  };

}

#endif