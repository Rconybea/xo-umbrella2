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
    return ', '.join(p['name'] for p in args)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', required=True, help='input IDL JSON5 file')
    parser.add_argument('--output', required=True, help='output directory')
    args = parser.parse_args()

    idl_fname = args.input
    idl = load_idl(idl_fname)
    output_dir = Path(args.output)
    output_dir.mkdir(parents=False, exist_ok=True)

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

    facet_includes = idl['includes']
    facet_ns1 = idl['namespace1']
    facet_ns2 = idl['namespace2']
    facet_name = idl['facet']          # e.g. Sequence
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
    iface_facet_any_fname = f'{iface_facet_any}.hpp'

    context = {
        'genfacet': __file__,
        'genfacet_input': args.input,
        #
        'facet_includes': facet_includes,
        'facet_ns1': facet_ns1,
        'facet_ns2': facet_ns2,
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
        'iface_facet_any_fname': iface_facet_any_fname,
        #
        'types': types,
        #
        'const_methods': const_methods,
        #
        'nonconst_methods': nonconst_methods,
    }

    # generate .hpp files

    templates = {}
    templates[abstract_facet_fname] = context['abstract_facet_hpp_j2']
    templates[iface_facet_any_fname] = context['iface_facet_any_hpp_j2']

    for output_file, template_name in templates.items():
        print(f'output_file: [{output_file}]')
        print(f'template_name: [{template_name}]')

        template = env.get_template(template_name)
        content = template.render(**context)

        (output_dir / output_file).write_text(content)
        print(f"Generated {output_dir}/{output_file}")


if __name__ == '__main__':
    main()
