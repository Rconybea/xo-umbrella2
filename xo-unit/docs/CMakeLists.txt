# xo-unit/docs/CMakeLists.txt

xo_doxygen_collect_deps()
xo_docdir_doxygen_config()
xo_docdir_sphinx_config(
    index.rst examples.rst glossary.rst install.rst implementation.rst
    quantity-reference.rst quantity-class.rst quantity-factoryfunctions.rst quantity-unitvars.rst quantity-source-code.rst
    xquantity-reference.rst xquantity-class.rst xquantity-source-code.rst
    scaled-unit-reference.rst scaled-unit-class.rst scaled-unit-constants.rst
    natural-unit-class.rst
    bpu-class.rst
    bu-store-class.rst basis-unit-reference.rst
    basis-unit-class.rst basis-unit-constants.rst
    dimension-enum.rst
    development.rst
    ubuntu-github-workflow.rst
)
