# xo-arena/docs/CMakeLists.txt

xo_doxygen_collect_deps()
xo_docdir_doxygen_config()
xo_docdir_sphinx_config(
    index.rst
    glossary.rst
    examples.rst
    implementation.rst
    #AAllocator-reference.rst
    #IAllocator_Xfer-reference.rst
    #AAllocIterator-reference.rst
    ArenaConfig-reference.rst
    DArena-reference.rst
    AllocInfo-reference.rst
    cmpresult-reference.rst
    #install.rst
    #introduction.rst
)

# see xo-reader/doc or xo-unit/doc for working examples
# example.rst install.rst implementation.rst
