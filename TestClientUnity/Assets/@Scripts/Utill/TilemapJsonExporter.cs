#if UNITY_EDITOR
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;
using UnityEngine.Tilemaps;

public static class TilemapJsonExporter
{
    // 충돌로 취급할 타일맵 이름들 (필요시 추가)
    private static readonly string[] BlockLayerNames = { "Collision", "Water", "Cliff","Portal" };
    private static readonly string[] PortalName = { "Portal" };
    [MenuItem("Tools/Export/Tilemap → JSON (0-based normalized)")]
    public static void ExportActiveSceneTilemapToJson()
    {
        // 1) 씬의 모든 Tilemap 수집 (비활성 제외: 필요하면 Include로 변경)
        var allTilemaps = UnityEngine.Object.FindObjectsByType<Tilemap>(
            FindObjectsInactive.Exclude, FindObjectsSortMode.None);

        if (allTilemaps == null || allTilemaps.Length == 0)
        {
            EditorUtility.DisplayDialog("Tilemap Export", "씬에 Tilemap이 없습니다.", "OK");
            return;
        }

        // 2) Grid 검색 & bounds 압축
        Grid grid = null;
        foreach (var tm in allTilemaps)
        {
            tm.CompressBounds(); // 지웠던 자취 제거
            grid ??= tm.GetComponentInParent<Grid>();
        }
        if (grid == null)
        {
            EditorUtility.DisplayDialog("Tilemap Export", "Grid를 찾지 못했습니다.", "OK");
            return;
        }

        // 3) Ground 계열만 골라서 맵 크기 정의 (이름이 Ground로 시작하는 타일맵)
        var groundMaps = allTilemaps.Where(tm => tm.name.StartsWith("Ground", StringComparison.OrdinalIgnoreCase)).ToList();
        if (groundMaps.Count == 0)
        {
            EditorUtility.DisplayDialog("Tilemap Export", "Ground* 타일맵을 찾지 못했습니다. 이름을 'Ground'로 시작하게 해주세요.", "OK");
            return;
        }

        // Ground 합집합 bounds
        BoundsInt total = groundMaps[0].cellBounds;
        for (int i = 1; i < groundMaps.Count; i++)
        {
            var b = groundMaps[i].cellBounds;
            total.xMin = Math.Min(total.xMin, b.xMin);
            total.yMin = Math.Min(total.yMin, b.yMin);
            total.xMax = Math.Max(total.xMax, b.xMax);
            total.yMax = Math.Max(total.yMax, b.yMax);
        }
        if (total.size.x <= 0 || total.size.y <= 0)
        {
            EditorUtility.DisplayDialog("Tilemap Export", "Ground 계열 bounds가 비어있습니다.", "OK");
            return;
        }

        int width = total.size.x;
        int height = total.size.y;

        // 4) 막힘 레이어들만 필터링
        var blockMaps = new List<Tilemap>();
        foreach (var tm in allTilemaps)
        {
            foreach (var name in BlockLayerNames)
            {
                if (tm.name.Equals(name, StringComparison.OrdinalIgnoreCase))
                {
                    blockMaps.Add(tm);
                    break;
                }
            }
        }

        // 5) passable(Top→Down) 생성 ? 0-based 정규화 (gx - total.xMin)
        var rows = new List<string>(height);
        for (int gy = total.yMax - 1; gy >= total.yMin; gy--)
        {
            var chars = new char[width];
            for (int gx = total.xMin; gx < total.xMax; gx++)
            {
                var cell = new Vector3Int(gx, gy, 0);
                bool blocked = false;
                foreach (var bm in blockMaps)
                {
                    if (bm != null && bm.HasTile(cell)) { blocked = true; break; }
                }
                chars[gx - total.xMin] = blocked ? '0' : '1'; // 0=막힘, 1=통과
            }
            rows.Add(new string(chars));
        }

        // 6) 스폰 포인트(태그): Spawn.Player / Spawn.Monster / Spawn.Item
        var spawns = new List<SpawnPoint>();
        var portals = new List<PortalPoint>();
        AppendSpawnsByTagWorld("Spawn.Player", "Player", spawns);
        AppendSpawnsByTagWorld("Spawn.Monster", "Monster", spawns);
        AppendSpawnsByTagWorld("Spawn.Item", "Item", spawns);
        AppendSpawnsByTagWorld("Spawn.Portal","Portal", spawns);
        float cellW = grid.cellSize.x;
        float cellH = grid.cellSize.y;
        Vector3 tlWorld = Vector3.zero;
        Vector3 brWorld = new Vector3(width * cellW, -height * cellH, 0f);
        int tlX = Mathf.RoundToInt(tlWorld.x);
        int tlY = Mathf.RoundToInt(tlWorld.y);
        int brX = Mathf.RoundToInt(brWorld.x);
        int brY = Mathf.RoundToInt(brWorld.y);
        // 7) DTO 구성 ? origin은 0,0으로 고정, 씬 오프셋은 mapOffset에 저장
        var dto = new MapDTO
        {
            version = 1,
            cellSize = new float[] { grid.cellSize.x, grid.cellSize.y },
            origin = new int[] { 0, 0 }, // JSON 내부 좌표계의 원점
            width = width,
            height = height,
            passableRowsTopDown = rows.ToArray(),
            spawns = spawns.ToArray(),
            portals = portals.ToArray(),
            mapOffset = new int[] { total.xMin, total.yMin },         // 씬(Grid) 기준 오프셋
            worldTopLeft = new int[] { tlX, tlY },
            worldBottomRight = new int[] { brX, brY },
        };

        // 8) 저장
        var path = EditorUtility.SaveFilePanel("Export Tilemap JSON (0-based)", Application.dataPath, "map_export.json", "json");
        if (string.IsNullOrEmpty(path)) return;

        var json = JsonUtility.ToJson(dto, true);
        File.WriteAllText(path, json);
        EditorUtility.DisplayDialog("Tilemap Export", $"완료!\n{path}", "OK");
    }

    private static void AppendSpawnsByTagWorld(string tag, string typeName, List<SpawnPoint> outList)
    {
        GameObject[] gos;
        try { gos = GameObject.FindGameObjectsWithTag(tag); }
        catch { return; } // 태그 없으면 무시

        foreach (var go in gos)
        {
            Vector3 pos = go.transform.position;
            outList.Add(new SpawnPoint
            {
                type = typeName,
                x = Mathf.RoundToInt(pos.x),
                y = Mathf.RoundToInt(pos.y)
            });
        }
    }
    private static void AppendPortalsByTagWorld(string tag, List<PortalPoint> outList)
    {
        GameObject[] gos;
        try { gos = GameObject.FindGameObjectsWithTag(tag); }
        catch { return; } // 태그 없으면 무시

        // 이름이 같은 포탈이 여러 개일 때 고유 id 보장
        var nameCount = new Dictionary<string, int>(StringComparer.Ordinal);

        // 출력 안정성을 위해 이름→위치순으로 정렬(선택)
        Array.Sort(gos, (a, b) =>
        {
            int cmp = string.Compare(a.name, b.name, StringComparison.Ordinal);
            if (cmp != 0) return cmp;
            // 이름이 같으면 좌표로 정렬
            var pa = a.transform.position; var pb = b.transform.position;
            cmp = pa.x.CompareTo(pb.x);
            return (cmp != 0) ? cmp : pa.y.CompareTo(pb.y);
        });

        foreach (var go in gos)
        {
            var p = go.transform.position;

            // 고유 id 만들기
            string baseName = go.name;
            if (!nameCount.TryGetValue(baseName, out int n)) n = 0;
            nameCount[baseName] = n + 1;
            string uniqueId = (n == 0) ? baseName : $"{baseName}_{n}";

            // 선택 메타: PortalMeta 컴포넌트가 있으면 읽기
            string destScene = null, destMapId = null, destPortalId = null;
            var meta = go.GetComponent<PortalMeta>(); // 없으면 null
            if (meta != null)
            {
                destScene = string.IsNullOrWhiteSpace(meta.DestScene) ? null : meta.DestScene;
                destMapId = string.IsNullOrWhiteSpace(meta.DestMapId) ? null : meta.DestMapId;
                destPortalId = string.IsNullOrWhiteSpace(meta.DestPortalId) ? null : meta.DestPortalId;
            }

            outList.Add(new PortalPoint
            {
                id = uniqueId,
                x = Mathf.RoundToInt(p.x),
                y = Mathf.RoundToInt(p.y),
                destScene = destScene,
                destMapId = destMapId,
                destPortalId = destPortalId
            });
        }
    }
    private static Vector3Int WorldToCell(Grid grid, Vector3 worldPos)
    {
        var localPos = grid.WorldToLocal(worldPos);
        return grid.LocalToCell(localPos);
    }

    [Serializable]
    private class MapDTO
    {
        public int version;
        public float[] cellSize;      // [x, y] (world size of one cell)
        public int[] origin;          // [0, 0] ? always 0-based in JSON
        public int width;             // number of columns
        public int height;            // number of rows
        public string[] passableRowsTopDown; // '1'=pass, '0'=block (top row first)
        public SpawnPoint[] spawns;   // normalized cell coords
        public PortalPoint[] portals;
        public int[] mapOffset;       // [minCellX, minCellY] in scene (for reverse mapping)
        public int[] worldTopLeft;     // [x, y] (int)
        public int[] worldBottomRight; // [x, y] (int)
    }

    [Serializable]
    private class SpawnPoint
    {
        public string type; // "Player", "Monster", "Item", ...
        public int x;       // normalized cell x (0..width-1)
        public int y;       // normalized cell y (0..height-1)
    }
    [Serializable]
    private class PortalPoint
    {
        public string id;   // 고유 식별자 (GameObject 이름 중복 시 자동 번호)
        public int x;       // 월드 X (int)
        public int y;       // 월드 Y (int)
                            // 선택 메타(있으면 내보냄)
        public string destScene;
        public string destMapId;
        public string destPortalId;
    }
}
#endif
