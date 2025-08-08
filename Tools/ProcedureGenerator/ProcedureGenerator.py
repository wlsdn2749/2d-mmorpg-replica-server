
import argparse
import jinja2
import XmlDBParser

def main():
    arg_parser = argparse.ArgumentParser(description = 'StoredProcedure Generator')
    arg_parser.add_argument('--path', type=str, default='C:/Users/wlsdn/workspace/iocp-server-unity-client-demo/iocp_server_cpp/GameServer/GameDB.xml', help='Xml Path')
    arg_parser.add_argument('--output', type=str, default='GenProcedures.h', help='Output File')
    args = arg_parser.parse_args()

    if args.path == None or args.output == None:
        print('[Error] --path --output required')
        return

    parser = XmlDBParser.XmlDBParser()
    parser.parse_xml(args.path)

    file_loader = jinja2.FileSystemLoader('Templates')
    env = jinja2.Environment(loader=file_loader)
    template = env.get_template('GenProcedures.h')

    output = template.render(procs=parser.procedures)
    f = open(args.output, 'w+')
    f.write(output)
    f.close()

    print(output)

if __name__ == '__main__':
    main()