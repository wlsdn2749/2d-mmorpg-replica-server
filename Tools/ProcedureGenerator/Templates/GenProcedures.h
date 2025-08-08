#pragma once
#include "Types.h"
#include <windows.h>
#include "DBBind.h"

{%- macro gen_procedures() -%} {% include 'Procedure.h' %} {% endmacro %}

namespace SP
{
	{{gen_procedures() | indent}}
};