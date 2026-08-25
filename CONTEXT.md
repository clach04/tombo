# Domain Model

## Glossary

  * **Expand All** -- Command that expands every folder node in the whole
    Tree View, regardless of the current selection. Paired with
    **Collapse All**, which collapses everything except the Root node.
  * **Collapse All** -- See Expand All.
  * **Expansion State** -- The set of expanded folder paths in the Tree View.
    Preserved across a Tree Refresh: nodes whose path was expanded before the
    refresh are re-expanded after it.

## Notes

  * Tree View population is eager: a Tree Refresh recursively inserts all
    folders and files up front; Expand All involves no filesystem work.
