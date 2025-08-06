using Microsoft.AspNetCore.DataProtection;
using Microsoft.Extensions.Configuration;
using Microsoft.IdentityModel.Tokens;
using System;
using System.Collections.Generic;
using System.IdentityModel.Tokens.Jwt;
using System.Linq;
using System.Security.Claims;
using System.Text;
using System.Threading.Tasks;

namespace AuthServer
{
   
    public interface IJwtTokenGenerator
    {
        string Generate(int userId, string email);
    }
    public class JwtTokenGenerator : IJwtTokenGenerator
    {
        private readonly IConfiguration _configuration;
        private readonly SymmetricSecurityKey _key;
        private string _secret;
        private int _expires; // 시간 단위 
        public JwtTokenGenerator(IConfiguration configuration)
        {
            _configuration  = configuration;
            _secret         = _configuration.GetValue<String>("AppSettings:JwtSecret");
            _expires        = _configuration.GetValue<int>("AppSettings:TokenExpiryHours");
            _key            = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(_secret));
        }

        public string Generate(int userId, string email)
        {
            var creds = new SigningCredentials(_key, SecurityAlgorithms.HmacSha256);
            var claims = new[]
            {
                new Claim(JwtRegisteredClaimNames.Sub, userId.ToString()),
                new Claim(JwtRegisteredClaimNames.UniqueName, email)
            };

            var token = new JwtSecurityToken(
                issuer: "GameAuth",
                audience: "GameClient",
                claims: claims,
                expires: DateTime.UtcNow.AddHours(_expires),
                signingCredentials: creds);


            return new JwtSecurityTokenHandler().WriteToken(token);

        }
    }
}
