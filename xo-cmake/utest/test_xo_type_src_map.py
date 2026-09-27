"""Unit tests for xo-type-src-map (xo-cmake/cmake/xo_macros/xo-type-src-map.py).

Builds a small synthetic source tree in a temporary directory -- one
"subsystem" plus one "other" subsystem it includes -- with a compile database
naming clang, runs the generator, and checks which types it maps and where.
Covers the cases .xo-backlog/xo-websock/issues/12 lists: nested and
out-of-line nested classes, templates and their specializations, enums,
aliases, anonymous namespaces, function-local classes, and types from
outside the subsystem; plus header TUs -- a header no TU includes is reached
through its one-line TU, a header a TU already includes is not dumped again,
and a header that does not compile on its own is a warning.

Skipped when no clang++ is on PATH (the generator needs one).
"""

import importlib.util
import json
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest

_HERE = pathlib.Path(__file__).resolve().parent
_SCRIPT = _HERE.parent / "cmake" / "xo_macros" / "xo-type-src-map.py"
_CLANG = shutil.which("clang++")

# line numbers below are 1-based lines of these texts: keep in step

OTHER_HPP = """\
namespace xo {
    namespace other {
        struct Lower { int x = 0; };
    }
}
"""

SUB_HPP = """\
#pragma once
#include "../../other/other.hpp"
namespace xo {
    namespace sub {
        class Plain {
        public:
            struct Nested { int y = 0; };
            struct OutOfLine;
        };

        template <typename T>
        class Box { T v; };

        template <>
        class Box<int> { int w; };

        enum class Colour { red, green };

        using PlainAlias = Plain;
    }
}
"""

SUB_CPP = """\
#include "sub.hpp"
namespace xo {
    namespace sub {
        struct Plain::OutOfLine {
            int z = 0;
        };

        namespace {
            struct Hidden { int h = 0; };
        }

        int f() {
            struct Local { int l = 0; };
            Local lo;
            Hidden hi;
            return lo.l + hi.h;
        }

        other::Lower lower() { return other::Lower{}; }
    }
}
"""

# included by no TU: reached only through its header TU
LONELY_HPP = """\
#pragma once
namespace xo {
    namespace sub {
        struct Lonely { int q = 0; };
    }
}
"""

# does not compile on its own
BROKEN_HPP = """\
#pragma once
static_assert(sizeof(Undeclared) > 0);
"""


@unittest.skipUnless(_CLANG, "clang++ not on PATH")
class TestTypeSourceMap(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.mkdtemp(prefix="xo-tsm-")
        root = pathlib.Path(cls.tmp)
        (root / "other").mkdir()
        (root / "sub" / "include").mkdir(parents=True)
        (root / "sub" / "src").mkdir()
        (root / "other" / "other.hpp").write_text(OTHER_HPP)
        (root / "sub" / "include" / "sub.hpp").write_text(SUB_HPP)
        (root / "sub" / "src" / "sub.cpp").write_text(SUB_CPP)
        (root / "sub" / "include" / "lonely.hpp").write_text(LONELY_HPP)
        (root / "sub" / "include" / "broken.hpp").write_text(BROKEN_HPP)

        src = root / "sub" / "src" / "sub.cpp"
        db = [{"directory": str(root / "sub"),
               "file": str(src),
               "command": f"{_CLANG} -std=c++20 -I{root / 'sub' / 'include'} -c {src} -o sub.o"}]

        # a header TU per header, as xo_type_source_map() generates them
        hdr_dir = root / "build" / "header-tus"
        hdr_dir.mkdir(parents=True)
        for h in ("sub.hpp", "lonely.hpp", "broken.hpp"):
            tu = hdr_dir / (h + ".cpp")
            tu.write_text(f'#include "{root / "sub" / "include" / h}"\n')
            db.append({"directory": str(root / "build"),
                       "file": str(tu),
                       "command": f"{_CLANG} -std=c++20 -I{root / 'sub' / 'include'}"
                                  f" -c {tu} -o {h}.o"})

        (root / "compile_commands.json").write_text(json.dumps(db))

        out = root / "types.json"
        r = subprocess.run([sys.executable, str(_SCRIPT),
                            "--compile-commands", str(root / "compile_commands.json"),
                            "--source-dir", str(root / "sub"),
                            "--repo-root", str(root),
                            "--header-tu-dir", str(hdr_dir),
                            "--clang", _CLANG,
                            "--output", str(out)],
                           capture_output=True, text=True)
        cls.run_result = r
        cls.map = json.loads(out.read_text()) if out.exists() else None

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tmp, ignore_errors=True)

    def types(self):
        self.assertEqual(self.run_result.returncode, 0, self.run_result.stderr)
        return self.map["types"]

    def test_header(self):
        self.types()
        self.assertEqual(self.map["format"], "xo-type-src-map/1")
        self.assertEqual(self.map["subsystem"], "sub")

    def test_namespace_class(self):
        self.assertEqual(self.types()["xo::sub::Plain"],
                         {"file": "sub/include/sub.hpp", "line": 5})

    def test_nested_class(self):
        self.assertEqual(self.types()["xo::sub::Plain::Nested"],
                         {"file": "sub/include/sub.hpp", "line": 7})

    def test_out_of_line_nested_definition_names_its_real_parent(self):
        t = self.types()
        self.assertEqual(t["xo::sub::Plain::OutOfLine"],
                         {"file": "sub/src/sub.cpp", "line": 4})
        self.assertNotIn("xo::sub::OutOfLine", t)

    def test_template_under_bare_name(self):
        self.assertEqual(self.types()["xo::sub::Box"],
                         {"file": "sub/include/sub.hpp", "line": 12})

    def test_specialization_is_not_a_separate_entry(self):
        # looks up by the template's bare name; the primary's location
        self.assertFalse([k for k in self.types() if "Box<" in k])

    def test_enum(self):
        self.assertEqual(self.types()["xo::sub::Colour"],
                         {"file": "sub/include/sub.hpp", "line": 17})

    def test_alias_is_not_an_entry(self):
        # type_name<T> of an alias spells the aliased type
        self.assertNotIn("xo::sub::PlainAlias", self.types())

    def test_anonymous_namespace_skipped(self):
        self.assertFalse([k for k in self.types() if "Hidden" in k])

    def test_function_local_class_skipped(self):
        self.assertFalse([k for k in self.types() if "Local" in k])

    def test_other_subsystems_types_not_included(self):
        # seen through an include, but defined outside --source-dir
        self.assertNotIn("xo::other::Lower", self.types())

    def test_header_no_tu_includes_is_reached_through_its_header_tu(self):
        self.assertEqual(self.types()["xo::sub::Lonely"],
                         {"file": "sub/include/lonely.hpp", "line": 4})

    def test_header_a_tu_includes_is_not_dumped_again(self):
        # sub.hpp is reached by sub.cpp: only lonely.hpp and broken.hpp alone
        self.types()
        self.assertIn("2 of 3 headers dumped alone", self.run_result.stderr)

    def test_header_not_self_contained_is_a_warning(self):
        self.types()
        self.assertIn("header does not compile on its own: sub/include/broken.hpp",
                      self.run_result.stderr)

    def test_exactly_these(self):
        self.assertEqual(sorted(self.types()),
                         ["xo::sub::Box", "xo::sub::Colour", "xo::sub::Lonely",
                          "xo::sub::Plain", "xo::sub::Plain::Nested",
                          "xo::sub::Plain::OutOfLine"])


def _load_module():
    spec = importlib.util.spec_from_file_location("xo_tsm", _SCRIPT)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


class TestParseDump(unittest.TestCase):
    """parse_dump on hand-written dump text: no clang needed"""

    def test_file_advances_on_a_non_decl_line(self):
        # clang prints a location's file only when it changed; the change may
        # be on a line that is no decl node (TemplateArgument: no address).
        # Regression: xo::nested::begin was attributed to the previous file
        m = _load_module()
        dump = "\n".join([
            "Dumping xo::nested:",
            "NamespaceDecl 0x1 </r/sub/a.hpp:1:1, line:40:1> line:1:11 nested",
            "|-CXXRecordDecl 0x2 <line:5:5, line:9:5> line:5:12 struct first definition",
            "| `-TemplateArgument </r/sub/b.hpp:60:38> type 'int'",
            "`-CXXRecordDecl 0x3 <line:83:5, line:87:5> line:83:12 struct begin definition",
        ])
        found = m.parse_dump(dump, "/r/sub")
        self.assertEqual(found["xo::nested::first"], ("/r/sub/a.hpp", 5))
        self.assertEqual(found["xo::nested::begin"], ("/r/sub/b.hpp", 83))

    def test_a_path_inside_a_type_string_is_not_a_location(self):
        # "(lambda at /file:line:col)" inside a quoted type is not a printed
        # location: clang's last-file does not move.  Regression: ppsink's
        # xo::nested::begin was attributed to xo-process's test .cpp
        m = _load_module()
        dump = "\n".join([
            "Dumping xo::nested:",
            "NamespaceDecl 0x1 </r/sub/a.hpp:1:1, line:40:1> line:1:11 nested",
            "|-DeclRefExpr 0x4 <col:20> 'Foo<(lambda at /r/other/t.cpp:9:5)>' lvalue",
            "`-CXXRecordDecl 0x3 <line:83:5, line:87:5> line:83:12 struct begin definition",
        ])
        found = m.parse_dump(dump, "/r/sub")
        self.assertEqual(found["xo::nested::begin"], ("/r/sub/a.hpp", 83))

    def test_gcc_internal_includes_dropped(self):
        m = _load_module()
        e = {"file": "/r/x.cpp", "directory": "/r",
             "command": "g++ -isystem /n/gcc-14/lib/gcc/x86_64-linux/14.3.0/include"
                        " -isystem /n/gcc-14/lib/gcc/x86_64-linux/14.3.0/include-fixed"
                        " -isystem /n/gcc-14/include/c++/14.3.0 -I/r/inc -c /r/x.cpp -o x.o"}
        self.assertEqual(m.tu_flags(e),
                         ["-isystem", "/n/gcc-14/include/c++/14.3.0", "-I/r/inc"])

    def test_no_compile_database_is_an_empty_map(self):
        # a header-only subsystem built on its own has no TUs, so cmake
        # writes no compile_commands.json.  Regression: the generator failed
        # the build (xo-subsys, xo-allocutil, xo-callback)
        m = _load_module()
        with tempfile.TemporaryDirectory() as d:
            out = os.path.join(d, "types.json")
            rc = m.main(["--compile-commands", os.path.join(d, "absent.json"),
                         "--source-dir", d, "--repo-root", d,
                         "--subsystem", "xo-hdronly", "--output", out])
            self.assertEqual(rc, 0)
            with open(out) as f:
                got = json.load(f)
            self.assertEqual(got["subsystem"], "xo-hdronly")
            self.assertEqual(got["types"], {})


if __name__ == "__main__":
    unittest.main()
