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
        # Extract FROM clause
        from_clause = query[from_idx+len('FROM'):]
        where_idx = max(from_clause.find('WHERE'), from_clause.find('where'))
        order_idx = max(from_clause.find('ORDER'), from_clause.find('order'))
        group_idx = max(from_clause.find('GROUP'), from_clause.find('group'))

        # Find end of FROM clause
        end_idx = len(from_clause)
        for idx in [where_idx, order_idx, group_idx]:
            if idx > 0 and idx < end_idx:
                end_idx = idx

        from_clause = from_clause[:end_idx].strip()

        # Extract all table names (support JOIN)
        used_tables = []
        tokens = from_clause.replace('[', '').replace(']', '').replace('dbo.', '').split()

        i = 0
        while i < len(tokens):
            token = tokens[i]
            # Skip JOIN keywords
            if token.upper() in ['INNER', 'LEFT', 'RIGHT', 'OUTER', 'FULL', 'CROSS', 'JOIN', 'ON', 'AND', 'OR', '=']:
                i += 1
                continue

            # Check if this is a table name
            if token in tables:
                used_tables.append(tables[token])
            i += 1

        # Parse SELECT clause columns
        select_clause = query[select_idx+len('SELECT'):from_idx].strip()
        words = select_clause.split(",")

        for word in words:
            # Remove alias from column name (e.g., "ce.characterId" -> "characterId")
            column_name = word.strip().split()[0]
            if '.' in column_name:
                column_name = column_name.split('.')[1]

            # Find column type from all used tables
            column_type = None
            for table in used_tables:
                if column_name in table.columns:
                    column_type = table.columns[column_name]
                    break

            if column_type:
                columns.append(Column(column_name, column_type))
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