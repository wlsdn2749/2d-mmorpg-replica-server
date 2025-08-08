import argparse
import jinja2
import ProtoParser

def main():
    
    arg_parser = argparse.ArgumentParser(description = 'PacketGenerator')
    arg_parser.add_argument('--path', type=str, default='C:/Users/wlsdn/source/repos/ServerCPP/Common/Protobuf/bin/Protocol.proto', help='proto path')
    arg_parser.add_argument('--output', type=str, default='TestPacketHandler', help='output file')
    arg_parser.add_argument('--recv', type=str, default='C_', help='Recv convention')
    arg_parser.add_argument('--send', type=str, default='S_', help='Send convention')
    arg_parser.add_argument('--ext', type=str, default='h', help='output file extension (e.g. h, cs')
    arg_parser.add_argument('--template', type=str, default='PacketHandler.h', help="which template do you use to make?")

    args = arg_parser.parse_args()

    parser = ProtoParser.ProtoParser(1000, args.recv, args.send)
    parser.parse_proto(args.path)

    # jinja2

    file_loader = jinja2.FileSystemLoader('Templates')
    env = jinja2.Environment(loader=file_loader)

    template = env.get_template(args.template) # 기본값은 PacketHandler.h 
    output = template.render(parser = parser, output=args.output)

    final_output = f"{args.output}.{args.ext}" # 기본값은 
    with open(final_output, 'w+', encoding='utf-8') as f:
        f.write(output)

    print(output)
    return;

if __name__ == '__main__':
    main()