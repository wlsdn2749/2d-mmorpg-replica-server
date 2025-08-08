import xml.etree.ElementTree as ET

class XmlDBParser:
    def __init__(self):
        self.tables = {}
        self.procedures = []

    def parse_xml(self, path):
        tree = ET.parse(path)
        root = tree.getroot()
        for child in root:
            if child.tag == 'Table':
                 self.tables[child.attrib['name']] = Table(child)
        for child in root:
            if child.tag == 'Procedure':
                self.procedures.append(Procedure(child, self.tables))

class Table:
    def __init__(self, node):
        self.name = node.attrib['name']
        self.columns = {}
        for child in node:
            if child.tag == 'Column':
                self.columns[child.attrib['name']] = ReplaceType(child.attrib['type'])

class Procedure:
    def __init__(self, node, tables):
        name = node.attrib['name']
        if name.startswith('sp'):
            self.name = name[2:]
        else:
            self.name = name
        self.params = []
        for child in node:
            if child.tag == 'Param':
                self.params.append(Param(child))
            elif child.tag == 'Body':
                self.columns = ParseColumns(child, tables)
                self.questions = MakeQuestions(self.params)

class Param:
    def __init__(self, node):
        name = node.attrib['name'].replace('@', '')
        self.name = name[0].upper() + name[1:]
        self.type = ReplaceType(node.attrib['type'])
        self.direction = node.attrib.get('dir', 'in')  # 기본값은 'in'
        self.is_output = self.direction == 'out'

class Column:
    def __init__(self, name, type):
        self.name = name[0].upper() + name[1:]
        self.type = type


def ParseColumns(node, tables):
    columns = []
    query = node.text
    select_idx = max(query.rfind('SELECT'), query.rfind('select'))
    from_idx = max(query.rfind('FROM'), query.rfind('from'))
    if select_idx > 0 and from_idx > 0 and from_idx > select_idx:
        table_name = query[from_idx+len('FROM') : -1].strip().split()[0]
        table_name = table_name.replace('[', '').replace(']', '').replace('dbo.', '')
        table = tables.get(table_name)
        words = query[select_idx+len('SELECT') : from_idx].strip().split(",")
        for word in words:
            column_name = word.strip().split()[0]
            columns.append(Column(column_name, table.columns[column_name]))
    elif select_idx > 0:
        word = query[select_idx+len('SELECT') : -1].strip().split()[0]
        if word.startswith('@@ROWCOUNT') or word.startswith('@@rowcount'):
            columns.append(Column('RowCount', 'int64'))
        elif word.startswith('@@IDENTITY') or word.startswith('@@identity'):
            columns.append(Column('Identity', 'int64'))
    return columns

def MakeQuestions(params):
    questions = ''
    if len(params) != 0:
        questions = '('
        for idx, item in enumerate(params):
            questions += '?'
            if idx != (len(params)-1):
                questions += ','
        questions += ')'
    return questions

def ReplaceType(type):
    if type == 'bool':
        return 'bool'
    if type == 'int':
        return 'int32'
    if type == 'bigint':
        return 'int64'
    if type == 'datetime':
        return 'TIMESTAMP_STRUCT'
    if type.startswith('nvarchar'):
        return 'nvarchar'
    if type.startswith('varbinary'):
        return 'varbinary'
    if type == 'tinyint':
        return 'int8'
    return type