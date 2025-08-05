using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AuthServer.Models
{
    public record User(int Id, string UserName, string PasswordHash);
}
