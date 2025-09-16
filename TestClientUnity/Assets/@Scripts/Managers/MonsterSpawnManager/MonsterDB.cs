using System.Collections.Generic;
using Newtonsoft.Json;

[System.Serializable]
public class MonsterDbDto
{
    [JsonProperty("version")] public int Version;
    [JsonProperty("monsters")] public List<MonsterDefDto> Monsters;
}

[System.Serializable]
public class MonsterDefDto
{
    [JsonProperty("monsterId")] public int Id;
    [JsonProperty("name")] public string Name;
    [JsonProperty("maxHp")] public int MaxHp;
    [JsonProperty("atk")] public int Atk;
}