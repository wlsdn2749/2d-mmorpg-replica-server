
using System;
using System.Collections.Generic;
using System.Linq;
using System.Data;
using System.Text;
using System.Threading.Tasks;

using Microsoft.Extensions.Configuration;
using Microsoft.Data.SqlClient;
using Microsoft.Data;
using Microsoft.AspNetCore.Http;

using AuthServer.Models;
using AuthServer.Context;
using Dapper;

namespace AuthServer.Contracts.Repository
{
    public interface IUserRepository
    {
        Task<bool> ExistAsync(string email); // SELECT: username이 있는지 확인
        Task CreateAsync(string email, string passwordHash); // INSERT: User를 생성해서 넣기
        Task<User?> FindAsync(string email); // SELECT: 만약 User가 없는경우? null 리턴
                                                             
    }
    public class UserRepository : IUserRepository
    {
        private readonly DapperContext _context;
        
        public UserRepository(DapperContext context)
        {
            _context = context;
        }

        public async Task<bool> ExistAsync(string email)
        {
            var query = "SELECT 1 FROM dbo.Users WHERE Email = @e";

            using (var connection = _context.CreateConnection())
            {
                var result = await connection.ExecuteScalarAsync<int>(query, new { e = email });
                return result == 1;
            }
        }

        public async Task CreateAsync(string email, string passwordHash)
        {
            var query = @"INSERT INTO dbo.Users (Email, PasswordHash)
                          VALUES (@Email, @passwordHash);";

            using (var connection = _context.CreateConnection())
            {
                await connection.ExecuteAsync(query, new 
                {   
                    Email = email,
                    PasswordHash = passwordHash
                });
            }
        }
 

        public async Task<User?> FindAsync(string email)
        {
            var query = @"SELECT Id, Email, PasswordHash FROM dbo.Users
                          WHERE Email = @e";

            using (var connection = _context.CreateConnection())
            {
                return await connection.QuerySingleOrDefaultAsync<User>(query, new
                {
                    e = email
                });
            }
        }
    }
}
