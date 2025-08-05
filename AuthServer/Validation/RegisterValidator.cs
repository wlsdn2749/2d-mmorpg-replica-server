using Mmorpg2d.Auth;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;





public static class RegisterValidator
{
    public readonly record struct ValidationResult(bool IsValid, string Detail)
    {
        public static ValidationResult Success() => new(true, "");
        public static ValidationResult Fail(string detail) => new(false, detail);
    }

    public static readonly Regex _korNickRx = new(@"^[가-힣]{2,6}$", RegexOptions.Compiled);

    public static async Task<ValidationResult> ValidateAsync(
        RegisterRequest req)
    {
        // 1. 한글만 인지
        // 2. username이 2~6글자가 아닌지
        if (!_korNickRx.IsMatch(req.Username))
            return ValidationResult.Fail("이름이 길거나 적합하지 않습니다.");

        // 3. 중복된 이름인지?
        if (await )

        return ValidationResult.Success();
    }
}

