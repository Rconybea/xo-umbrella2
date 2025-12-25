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
    env.filters['argnames'] = format_arg_names

    facet_includes = idl['includes']
    facet_ns1 = idl['namespace1']
    facet_ns2 = idl['namespace2']
    facet_name = idl['facet']          # e.g. Sequence
    facet_brief = idl['brief']
    facet_doc = '\n'.join(idl['doc'])
    methods = idl['methods']
    for method in methods:
        method['doc'] = '\n'.join(method['doc'])

    abstract_facet = f'A{facet_name}'
    abstract_facet_fname = f'{abstract_facet}.hpp'

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
        'methods': methods
    }

    # generate .hpp files

    templates = {}
    templates[abstract_facet_fname] = context['abstract_facet_hpp_j2']

    for output_file, template_name in templates.items():
        print(f'output_file: [{output_file}]')
        print(f'template_name: [{template_name}]')

        template = env.get_template(template_name)
        content = template.render(**context)
        
        (output_dir / output_file).write_text(content)
        print(f"Generated {output_dir}/{output_file}")


if __name__ == '__main__':
    main()
    
