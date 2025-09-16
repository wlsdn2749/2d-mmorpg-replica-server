using Google.Apis.Auth.OAuth2;
using Google.Apis.Sheets.v4;
using Google.Apis.Sheets.v4.Data;
using Google.Apis.Services;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using System.Linq;

namespace SheetLoad
{
    public class SheetManager
    {
        #region Singleton, Ctor
        //private 생성자 
        private SheetManager() { }

        //private static 인스턴스 객체
        private static readonly Lazy<SheetManager> _instance = new Lazy<SheetManager>(() => new SheetManager());

        //public static 의 객체반환 함수
        public static SheetManager Instance { get { return _instance.Value; } }

        #endregion

        // 고정 상수값
        static readonly string SpreadsheetId = "1n3NPDcOekTDOeW_idZ7FKkTUoocsTL0J835ySUD_29I"; // 시트 URL
        static readonly string credFile = "service-account-key.json";

        // JSON 직렬화 옵션 캐시
        private static readonly JsonSerializerOptions _jsonOptions = new JsonSerializerOptions
        {
            WriteIndented = true,
            PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
            Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping
        };

        // 파일 경로 관리
        private static string GetResourcesPath()
        {
            // 실행 위치에서 상위로 올라가서 원본 소스 위치의 resources 폴더 찾기
            string currentDir = AppContext.BaseDirectory;

            // bin/Debug/Net9.0/ 에서 4단계 위로 올라가면 SheetLoader 폴더
            string projectRoot = Directory.GetParent(currentDir)?.Parent?.Parent?.Parent?.FullName;

            if (projectRoot != null)
            {
                string resourcesDir = Path.Combine(projectRoot, "resources");
                Directory.CreateDirectory(resourcesDir);
                return resourcesDir;
            }

            // fallback: 기존 방식
            string fallbackDir = Path.Combine(AppContext.BaseDirectory, "resources");
            Directory.CreateDirectory(fallbackDir);
            return fallbackDir;
        }

        private SheetsService _sheetService;
        public void init()
        {
            _sheetService = InitSheetsService();
        }

        private SheetsService InitSheetsService()
        {
            GoogleCredential credential = GoogleCredential
                .FromFile(credFile)
                .CreateScoped(SheetsService.Scope.Spreadsheets);

            return new SheetsService(new BaseClientService.Initializer()
            {
                HttpClientInitializer = credential,
                ApplicationName = "Game Sheet",
            });
        }

        public SheetsService GetSheetsService()
        {
            return _sheetService;
        }

        public IList<IList<object>> GetData(string sheetName, string start, string end)
        {
            string query = $"{sheetName}!{start}:{end}"; 
            var request = _sheetService.Spreadsheets.Values.Get(SpreadsheetId, query);
            var response = request.Execute();

            if (response.Values != null)
            {
                return response.Values;
            }

            throw new Exception("Parse Error! Check Query");
            //return new List<IList<object>>(); // 빈 리스트 반환
        }

        public string GetDataAsJson(string sheetName)
        {
            // 1. 먼저 헤더 행을 읽어서 실제 키 개수 파악
            var headers = GetHeaders(sheetName);
            if (headers.Count == 0)
            {
                return "[]";
            }

            // 2. 헤더 개수에 맞춰 전체 데이터 범위 설정
            char lastColumn = (char)('A' + headers.Count - 1);
            string dataRange = $"A1:{lastColumn}1000"; // 충분히 큰 범위로 설정

            var allData = GetData(sheetName, "A1", $"{lastColumn}1000");

            // 3. 첫 번째 행(헤더) 제외하고 데이터 행들만 처리
            var jsonArray = new List<Dictionary<string, object>>();

            for (int i = 1; i < allData.Count; i++) // 인덱스 1부터 시작 (헤더 제외)
            {
                var row = allData[i];
                var rowDict = new Dictionary<string, object>();

                // 빈 행 체크 (모든 셀이 비어있으면 종료)
                if (row.All(cell => string.IsNullOrWhiteSpace(cell?.ToString())))
                {
                    break;
                }

                // 각 헤더에 대응하는 값 매핑 (camelCase로 변환)
                for (int j = 0; j < headers.Count; j++)
                {
                    string value = j < row.Count ? row[j]?.ToString() ?? "" : "";
                    string camelCaseKey = ToCamelCase(headers[j]);
                    rowDict[camelCaseKey] = value;
                }

                jsonArray.Add(rowDict);
            }

            return JsonSerializer.Serialize(jsonArray, _jsonOptions);
        }

        private List<string> GetHeaders(string sheetName)
        {
            // 넓은 범위로 첫 번째 행 읽기
            var headerRow = GetData(sheetName, "A1", "ZZ1");
            var headers = new List<string>();

            if (headerRow.Count > 0)
            {
                var firstRow = headerRow[0];
                for (int i = 0; i < firstRow.Count; i++)
                {
                    string header = firstRow[i]?.ToString()?.Trim();
                    if (string.IsNullOrWhiteSpace(header))
                    {
                        break; // 빈 헤더가 나오면 중단
                    }
                    headers.Add(header);
                }
            }

            return headers;
        }

        private static string ToCamelCase(string input)
        {
            if (string.IsNullOrWhiteSpace(input) || input.Length == 0)
                return input;

            // 첫 글자만 소문자로 변경
            return char.ToLower(input[0]) + input.Substring(1);
        }

        public string SaveDataAsJson(string sheetName)
        {
            string jsonData = GetDataAsJson(sheetName);
            string fileName = Path.Combine(GetResourcesPath(), $"{sheetName}_data.json");
            File.WriteAllText(fileName, jsonData);
            return fileName;
        }
    }
}
