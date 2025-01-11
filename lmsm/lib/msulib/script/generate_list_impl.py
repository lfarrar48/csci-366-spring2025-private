import argparse
import os
import sys

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

try:
    import chevron
except ImportError:
    os.system("pip install ")
    import chevron

# disable html escapes since we're using C (lol)
chevron.render.__globals__['_html_escape'] = lambda s: s

def render(path, values):
    if not os.path.isabs(path):
        path = os.path.join(BASE_DIR, path)
    with open(path, 'r') as f:
        return chevron.render(f, values)

if __name__ == '__main__':
    print('\n'.join(sys.argv), '\n')
    print('\n'.join(sys.orig_argv), '\n')
    parser = argparse.ArgumentParser('list_gen')
    parser.add_argument('--element-type', dest='typename', required=True, help='the type you want to make a list for')
    parser.add_argument('--list-name', dest='list_name', required=True, help='the name of the list')
    parser.add_argument('--hpath', dest='header_path', required=True, help='the path to the header file')
    parser.add_argument('--cpath', dest='source_path', required=True, help='the path to the source file')

    parser.add_argument('--hinclude', dest='hincludes', action='append', help='include items for the header')
    parser.add_argument('--hdef', dest='hdefs', action='append', help='a list of definitions to add to the header')

    parser.add_argument('--cinclude', dest='cincludes', action='append', help='include items for the source')
    parser.add_argument('--cdef', dest='cdefs', action='append', help='a list of definitions for the source file')

    parser.add_argument('--free-fn', dest='free', help='a free function for the free impl')
    parser.add_argument('--hash-fn', dest='hash', help='a hash function for the hash impl')
    parser.add_argument('--eq-fn', dest='eq', help='an equality function for the contains impl')

    args = parser.parse_args()

    data = {
        'label': args.list_name.replace("[^\w_]+", "_").upper(),
        'typename': args.typename.replace("\s+", "_"),
        'list_name': args.list_name + "_t",
        'struct_name': args.list_name,
        'header_path': args.header_path,
        'source_path': args.source_path,

        'hincludes': args.hincludes,
        'hdefs': args.hdefs,

        'cincludes': args.cincludes,
        'cdefs': args.cdefs,

        'eq': args.eq,
        'free': args.free,
        'hash': args.hash,
    }

    with open(args.header_path, "w") as f:
        content = render('list.h.mustache', data)
        print(content)
        f.write(content)

    with open(args.source_path, "w") as f:
        content = render('list.c.mustache', data)
        print(content)
        f.write(content)
