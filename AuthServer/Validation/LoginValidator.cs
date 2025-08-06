using AuthServer.Contracts.Repository;
using Mmorpg2d.Auth;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

public static class LoginValidator
{
    public readonly record struct ValidationResult(bool IsValid, string Detail)
    {
        public static ValidationResult Success() => new(true, "");
        public static ValidationResult Fail(string detail) => new(false, detail);
    }

    //public static readonly Regex _korNickRx = new(@"^[가-힣]{2,6}$", RegexOptions.Compiled);
    public static readonly Regex _emailRx = new(@"^[^@\s]+@[^@\s]+\.[^@\s]+$", RegexOptions.Compiled | RegexOptions.IgnoreCase);

    public static async Task<ValidationResult> ValidateAsync(
        LoginRequest req,
        IUserRepository userRepository)
    {
        // 1. 공백 or null 체크
        if (string.IsNullOrWhiteSpace(req.Email) ||
           string.IsNullOrWhiteSpace(req.Password))
            return ValidationResult.Fail("Username or Password는 필수 항목입니다.");

        // 2. 존재 여부 확인
        if (!await userRepository.ExistAsync(req.Email))
            return ValidationResult.Fail("존재하지 않는 계정입니다.");

        return ValidationResult.Success();
    }
}

