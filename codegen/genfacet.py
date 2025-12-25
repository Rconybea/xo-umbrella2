#! /usr/bin/env python3
#
# genfacet.py

import json5
import argparse
from pathlib import Path
from jinja2 import Environment, FileSystemLoader

def load_idl(path):
    with open(path) as f:
        return json5.load(f)

def format_method_qualifiers(method):
    """ Build qualifier string: const noexcept
    """
    quals = []
    if method.get('const', False):
        quals.append('const')
    if method.get('noexcept', False):
        quals.append('noexcept')

    return ' '.join(quals)

def format_args(args, include_names=True):
    """ Format argument list for a method
    """
    if not args:
        return ''
    if include_names:
        return ', '.join(f"{p['type']} {p['name']}" for p in args)
    else:
        return ', '.join(p['type'] for p in args)

def format_args_nonames(args):
    return format_args(args, False)

def format_arg_names(args):
    """ Format argument names, for forwarding
    """
    names = [p['name'] for p in args]
    return ', '.join([f"_dcast({names[0]})"] + names[1:])

def format_args_nodata(args):
    """ Format arguments, but exclude data arg
    """
    return format_args(args[1:])

def format_args_routing(args):
    """ Format argument names, for routing
    """
    names = [p['name'] for p in args]
    return ', '.join([f"O::data()"] + names[1:])

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', required=True, help='input IDL JSON5 file')
    parser.add_argument('--output-hpp', required=True, help='.hpp output directory')
    parser.add_argument('--output-cpp', required=True, help='.cpp output directory')

    args = parser.parse_args()

    idl_fname = args.input
    idl = load_idl(idl_fname)
    #
    output_hpp_dir = Path(args.output_hpp)
    output_hpp_dir.mkdir(parents=False, exist_ok=True)
    #
    output_cpp_dir = Path(args.output_cpp)
    output_cpp_dir.mkdir(parents=False, exist_ok=True)

    # setup jinja2
    template_dir = Path(__file__).parent
    #template_dir = Path(__file__).parent / 'codegen'

    print(f'template_dir: [{template_dir}]')

    env = Environment(loader = FileSystemLoader(template_dir),
                      trim_blocks = True,
                      lstrip_blocks = True)

    # custom filters
    env.filters['qualifiers'] = format_method_qualifiers
    env.filters['args'] = format_args
    env.filters['argtypes'] = format_args_nonames
    env.filters['argnames'] = format_arg_names
    env.filters['argsnodata'] = format_args_nodata
    env.filters['argrouting'] = format_args_routing

    # true to insert doxygen markup in generated .hpp/.cpp files
    using_dox = idl['using_doxygen']

    facet_includes = idl['includes']
    facet_ns1 = idl['namespace1']
    facet_ns2 = idl['namespace2']
    facet_name = idl['facet']          # e.g. Sequence
    facet_name_lc = facet_name.lower()
    facet_brief = idl['brief']
    facet_doc = '\n'.join(idl['doc'])

    types = idl['types']
    for ty in types:
        ty['doc'] = '\n'.join(ty['doc'])

    const_methods = idl['const_methods']
    for md in const_methods:
        md['args'] = [{'type': "Copaque", 'name': "data"}] + md['args']
        md['doc'] = '\n'.join(md['doc'])

    nonconst_methods = idl['nonconst_methods']
    for md in nonconst_methods:
        md['args'] = [{'type': "Opaque", 'name': "data"}] + md['args']
        md['doc'] = '\n'.join(md['doc'])

    abstract_facet = f'A{facet_name}'
    abstract_facet_fname = f'{abstract_facet}.hpp'
    #
    iface_facet = f'I{facet_name}'
    iface_facet_impltype = f'{iface_facet}_ImplType'
    #
    iface_facet_any = f'{iface_facet}_Any'
    iface_facet_any_hpp_fname = f'{iface_facet_any}.hpp'
    iface_facet_any_cpp_fname = f'{iface_facet_any}.cpp'
    #
    iface_facet_xfer = f'{iface_facet}_Xfer'
    iface_facet_xfer_hpp_fname = f'{iface_facet_xfer}.hpp'
    iface_facet_xfer_cpp_fname = f'{iface_facet_xfer}.cpp'
    #
    router_facet = f'R{facet_name}'
    router_facet_hpp_fname = f'{router_facet}.hpp'

    context = {
        'genfacet': __file__,
        'genfacet_input': args.input,
        'using_dox': using_dox,
        #
        'facet_includes': facet_includes,
        'facet_ns1': facet_ns1,
        'facet_ns2': facet_ns2,
        'facet_name_lc': facet_name_lc,
        #'name': facet_name,
        'idl_fname': idl_fname,
        #
        'abstract_facet_hpp_j2': 'abstract_facet.hpp.j2',
        'abstract_facet': abstract_facet,
        'abstract_facet_fname': abstract_facet_fname,
        'abstract_facet_doc': facet_doc,
        #
        'iface_facet': iface_facet,
        'iface_facet_impltype': iface_facet_impltype,
        #
        'iface_facet_any': iface_facet_any,
        'iface_facet_any_hpp_j2': 'iface_facet_any.hpp.j2',
        'iface_facet_any_cpp_j2': 'iface_facet_any.cpp.j2',
        'iface_facet_any_hpp_fname': iface_facet_any_hpp_fname,
        'iface_facet_any_cpp_fname': iface_facet_any_cpp_fname,
        #
        'iface_facet_xfer': iface_facet_xfer,
        'iface_facet_xfer_hpp_j2': 'iface_facet_xfer.hpp.j2',
        'iface_facet_xfer_cpp_j2': 'iface_facet_xfer.cpp.j2',
        'iface_facet_xfer_hpp_fname': iface_facet_xfer_hpp_fname,
        'iface_facet_xfer_cpp_fname': iface_facet_xfer_cpp_fname,
        #
        'router_facet': router_facet,
        'router_facet_hpp_j2': 'router_facet.hpp.j2',
        'router_facet_hpp_fname': router_facet_hpp_fname,
        #
        'types': types,
        #
        'const_methods': const_methods,
        #
        'nonconst_methods': nonconst_methods,
    }

    # generate .hpp files

    templates = {}
    templates[abstract_facet_fname] = [output_hpp_dir,
                                       context['abstract_facet_hpp_j2']]
    templates[iface_facet_any_hpp_fname] = [output_hpp_dir,
                                            context['iface_facet_any_hpp_j2']]
    templates[iface_facet_any_cpp_fname] = [output_cpp_dir,
                                            context['iface_facet_any_cpp_j2']]
    templates[iface_facet_xfer_hpp_fname] = [output_hpp_dir,
                                             context['iface_facet_xfer_hpp_j2']]
    templates[router_facet_hpp_fname] = [output_hpp_dir,
                                         context['router_facet_hpp_j2']]

    for out_file, record in templates.items():
        out_dir = record[0]
        template_name = record[1]

        print(f'out_dir: [{out_dir}]')
        print(f'out_file: [{out_file}]')
        print(f'template_name: [{template_name}]')

        template = env.get_template(template_name)
        content = template.render(**context)

        (out_dir / out_file).write_text(content)
        print(f"Generated {out_dir}/{out_file}")


if __name__ == '__main__':
    main()
