#if UNITY_EDITOR
using System;
using System.IO;
using System.Text;
using System.Diagnostics;
using System.Linq;
using UnityEditor;
using UnityEngine;
public class AIContextPackager : EditorWindow
{
    private enum Scope { SelectedAsset, SelectedFolder, GitDiffSinceHEAD, EntireProjectScripts }
    private Scope scope = Scope.SelectedAsset;
    private string taskInstruction = "ex)가독성과 성능을 위해 코드를 리팩토링 해줘. 동작은 동일하게 유지되어야 해.";
    private int maxChars = 180000; // Chat input 한도 대비 여유치
    private string exportPath = "Assets/AI_Helper_Context.txt";
    private Vector2 scroll;
    private string preview;


    [MenuItem("Tools/AI Helper/Context Packager")] private static void Open() => GetWindow<AIContextPackager>(true, "AI Context Packager").minSize = new Vector2(720, 520);


    private void OnGUI()
    {
        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Context Packager (copy & paste to ChatGPT)", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(" -Scope- " +
            "\n1.Selected Asset :Project 창에서 선택한 파일 1~N개만 포함." +
            "\n2.Selected Folder : 선택한 폴더 안의 모든 .cs 재귀 수집. " +
            "\n3.Git Diff Since HEAD : 마지막 커밋 이후 변경된 C# 파일만 포함" +
            "\n4.Entire Project Scripts : Assets 아래 모든 .cs 전체 포함.(길어서 모델 부담 커짐.)" +
            "\n\n - Task 및 사용법 -" +
            "\nTask 칸에 원하는 지시 작성 (예: “성능 유지하며 가독성 개선, 메모리 할당 줄여줘”). " +
            "\n요청 사항을 적고 Preview를 누른 뒤 복사하여 GPT에 붙여넣기.", MessageType.Info);


        scope = (Scope)EditorGUILayout.EnumPopup("Scope", scope);
        taskInstruction = EditorGUILayout.TextField("Task", taskInstruction);
        maxChars = EditorGUILayout.IntField("Max Characters", maxChars);
        EditorGUILayout.Space();


        using (new EditorGUILayout.HorizontalScope())
        {
            if (GUILayout.Button("Preview"))
            {
                preview = BuildContext();
                if (preview.Length > maxChars) preview = preview.Substring(0, maxChars) + "\n...[TRUNCATED]";
            }
            if (GUILayout.Button("Copy to Clipboard"))
            {
                GUIUtility.systemCopyBuffer = BuildContext().Substring(0, Mathf.Min(maxChars, BuildContext().Length));
                EditorUtility.DisplayDialog("AI Context", "Copied to clipboard.", "OK");
            }
            if (GUILayout.Button("Export .txt"))
            {
                var path = EditorUtility.SaveFilePanel("Export Context", Application.dataPath, "AI_Helper_Context", "txt");
                if (!string.IsNullOrEmpty(path)) File.WriteAllText(path, BuildContext().Substring(0, Mathf.Min(maxChars, BuildContext().Length)), new UTF8Encoding(false));
            }
        }


        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Preview", EditorStyles.boldLabel);
        scroll = EditorGUILayout.BeginScrollView(scroll);
        EditorGUILayout.TextArea(preview ?? "(Click Preview)", GUILayout.ExpandHeight(true));
        EditorGUILayout.EndScrollView();
    }
    private string BuildContext()
    {
        var sb = new StringBuilder();
        sb.AppendLine("# Task\n" + taskInstruction + "\n");
        sb.AppendLine("# Project: " + PlayerSettings.productName + " (Unity " + Application.unityVersion + ")\n");
        sb.AppendLine("# Scope: " + scope + "\n");


        switch (scope)
        {
            case Scope.SelectedAsset:
                foreach (var path in Selection.assetGUIDs.Select(AssetDatabase.GUIDToAssetPath))
                    AppendFileIfCode(sb, path);
                break;
            case Scope.SelectedFolder:
                foreach (var path in Selection.assetGUIDs.Select(AssetDatabase.GUIDToAssetPath))
                    AppendFolderRecursive(sb, path);
                break;
            case Scope.GitDiffSinceHEAD:
                AppendGitDiff(sb);
                break;
            case Scope.EntireProjectScripts:
                foreach (var path in Directory.GetFiles(Application.dataPath, "*.cs", SearchOption.AllDirectories))
                    AppendFile(sb, path);
                break;
        }
        return sb.ToString();
    }
    private void AppendFolderRecursive(StringBuilder sb, string path)
    {
        if (File.Exists(path)) { AppendFileIfCode(sb, path); return; }
        var abs = Path.GetFullPath(path);
        foreach (var file in Directory.GetFiles(abs, "*.cs", SearchOption.AllDirectories)) AppendFile(sb, file);
        foreach (var shader in Directory.GetFiles(abs, "*.shader", SearchOption.AllDirectories)) AppendFile(sb, shader);
    }


    private void AppendFileIfCode(StringBuilder sb, string relativePath)
    {
        var abs = Path.GetFullPath(relativePath);
        if (abs.EndsWith(".cs") || abs.EndsWith(".shader") || abs.EndsWith(".uxml") || abs.EndsWith(".uss"))
            AppendFile(sb, abs);
    }
    private void AppendFile(StringBuilder sb, string absPath)
    {
        try
        {
            var text = File.ReadAllText(absPath);
            var rel = MakeProjectRelative(absPath);
            sb.AppendLine($"\n--- FILE: {rel} ---\n");
            sb.AppendLine("```" + GetFence(absPath));
            sb.AppendLine(text);
            sb.AppendLine("```");
        }
        catch (Exception e) { UnityEngine.Debug.LogWarning($"AIContextPackager: {e.Message}"); }
    }


    private void AppendGitDiff(StringBuilder sb)
    {
        try
        {
            var psi = new ProcessStartInfo("git", "diff --name-only HEAD") { WorkingDirectory = Directory.GetCurrentDirectory(), RedirectStandardOutput = true, UseShellExecute = false, CreateNoWindow = true };
            using var p = Process.Start(psi);
            var list = p.StandardOutput.ReadToEnd().Split(new[] { '\n', '\r' }, StringSplitOptions.RemoveEmptyEntries);
            foreach (var rel in list.Where(f => f.EndsWith(".cs") || f.EndsWith(".shader") || f.EndsWith(".uxml") || f.EndsWith(".uss")))
            {
                var abs = Path.GetFullPath(rel);
                AppendFile(sb, abs);
            }
        }
        catch (Exception e) { UnityEngine.Debug.LogWarning($"AIContextPackager git diff failed: {e.Message}"); }
    }
    private string MakeProjectRelative(string abs) => abs.Replace(Path.GetFullPath(Directory.GetCurrentDirectory()) + Path.DirectorySeparatorChar, string.Empty);
    private string GetFence(string path)
    {
        if (path.EndsWith(".cs")) return "csharp";
        if (path.EndsWith(".shader")) return "hlsl";
        if (path.EndsWith(".uxml")) return "xml";
        if (path.EndsWith(".uss")) return "css";
        return "";
    }
}
#endif