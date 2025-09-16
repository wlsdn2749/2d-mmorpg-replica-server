using Google.Apis.Auth.OAuth2;
using Google.Apis.Services;
using Google.Apis.Sheets.v4;
using Google.Apis.Sheets.v4.Data;
using SheetLoad;
using System;
using System.Collections.Generic;
using System.IO;
using static Google.Apis.Requests.BatchRequest;

namespace SimpleGoogleSheetsExample
{
    class Program
    {
        // 여기에 실제 스프레드시트 ID 입력
        static SheetManager? _sheetManager;
        static List<string> sheetNameList = new List<string>{
            "Monster", "SpawnPoint"
        };

        static void Main(string[] args)
        {
            _sheetManager = SheetManager.Instance;
            _sheetManager.init();

            foreach (string sheetName in sheetNameList)
            {
                try
                {
                    Console.WriteLine($"=== {sheetName} 시트 데이터 처리 ===");

                    // JSON 변환 및 저장
                    string savedFilePath = _sheetManager.SaveDataAsJson(sheetName);
                    Console.WriteLine($"JSON 파일이 저장되었습니다: {savedFilePath}");

                    // JSON 내용 출력 (선택적)
                    if (args.Length > 0 && args[0] == "--verbose")
                    {
                        Console.WriteLine("\n=== JSON 내용 ===");
                        string jsonResult = _sheetManager.GetDataAsJson(sheetName);
                        Console.WriteLine(jsonResult);
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"오류 발생: {ex.Message}");
                }
            }
        }
    }
}