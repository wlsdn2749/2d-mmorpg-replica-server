
using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using AuthServer.Models;
using Microsoft.Extensions.Configuration;
using Microsoft.Data.SqlClient;

namespace AuthServer.Data
{
    internal interface IUserRepositiory
    {
        Task<bool> ExistAsync(string username); // SELECT: username이 있는지 확인
        Task CreateAsync(string username, string passwordHash); // INSERT: User를 생성해서 넣기
        Task<User?> FindAsync(string username); // SELECT: 만약 User가 없는경우? null 리턴
                                                             
    }
    internal class UserRepository : IUserRepositiory
    {
        private readonly IDbConnection _db;
        public UserRepository(IConfiguration cfg)
            => _db = new SqlConnection(cfg.GetConnectionString("..."));
        public Task CreateAsync(string username, string passwordHash)
        {
            throw new NotImplementedException();
        }

        public Task<bool> ExistAsync(string username)
        {
            throw new NotImplementedException();
        }

        public Task<User?> FindAsync(string username)
        {
            throw new NotImplementedException();
        }
    }
}
