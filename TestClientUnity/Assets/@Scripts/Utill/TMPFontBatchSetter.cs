// Assets/Editor/TMPFontBatchSetter.cs
#if UNITY_EDITOR
using UnityEngine;
using UnityEditor;
using TMPro;

public static class TMPFontBatchSetter
{
    [MenuItem("Tools/TMP/Apply Font Asset To All In Scene")]
    static void ApplyToAllInScene()
    {
        var font = Selection.activeObject as TMP_FontAsset;
        if (!font)
        {
            EditorUtility.DisplayDialog("TMP", "Project 창에서 적용할 TMP_FontAsset를 선택한 뒤 다시 실행하세요.", "OK");
            return;
        }

        int count = 0;
        foreach (var text in Object.FindObjectsByType<TMP_Text>(FindObjectsSortMode.None))
        {
            Undo.RecordObject(text, "Assign TMP Font");
            text.font = font;
            text.fontMaterial = font.material; // 안전하게 폰트의 기본 머티리얼로
            EditorUtility.SetDirty(text);
            count++;
        }
        Debug.Log($"[TMP] 적용 완료: {count}개 TMP_Text에 '{font.name}' 할당");
    }
}
#endif
