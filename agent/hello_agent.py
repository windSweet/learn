import requests
import os
from tavily import TavilyClient
from openai import OpenAI
import re
import anthropic

AGENT_SYSTEM_PROMPT ="""
你是⼀个智能旅⾏助⼿。你的任务是分析⽤户的请求，并使⽤可⽤⼯具⼀步步地解决问题。
# 可⽤⼯具:
- `get_weather(city: str)`: 查询指定城市的实时天⽓。
- `get_attraction(city: str, weather: str)`: 根据城市和天⽓搜索推荐的旅游景点。

# 输出格式要求:
你的每次回复必须严格遵循以下格式,包含⼀对Thought和Action:
Thought:[你的思考过程和下⼀步计划]
Action:[你要执⾏的具体⾏动]

Action的格式必须是以下之⼀:
1. 调⽤⼯具:function_name(arg_name="arg_value")
2. 结束任务:Finish[最终答案]
# 重要提示:
- 每次只输出⼀对Thought-Action
- Action必须在同⼀⾏,不要换⾏
- 当收集到⾜够信息可以回答⽤户问题时,必须使⽤Action: Finish[最终答案] 
格式结束
请开始吧！
"""

API_KEY = "sk-thOrxkujMWBoBqzUk5qGh76dM1nqAvp2PLpvwxnH1Q4wDwUh"
BASE_URL = "https://www.dogapi.cc/v1"
MODEL = "gpt-5.6-sol"


WMO_CODE_MAP = {
    0: "晴", 1: "基本晴朗", 2: "部分多云", 3: "阴",
    45: "雾", 48: "雾凇",
    51: "小毛毛雨", 53: "毛毛雨", 55: "大毛毛雨",
    61: "小雨", 63: "中雨", 65: "大雨",
    71: "小雪", 73: "中雪", 75: "大雪",
    95: "雷暴", 96: "雷暴伴小冰雹", 99: "雷暴伴大冰雹"
}

def get_weather(city: str) -> str:
    # ========== 第一步：地理编码，获取经纬度 ==========
    geo_url = "https://geocoding-api.open-meteo.com/v1/search"
    geo_params = {
        "name": city,
        "count": 1,
        "language": "zh",
        "format": "json"
    }
    
    geo_resp = requests.get(geo_url, params=geo_params, timeout=10)
    geo_data = geo_resp.json()
    
    # 检查是否找到城市
    if "results" not in geo_data or not geo_data["results"]:
        return f"未找到城市: {city}"
    
    # 提取经纬度和时区
    lat = geo_data["results"][0]["latitude"]
    lon = geo_data["results"][0]["longitude"]
    tz = geo_data["results"][0]["timezone"]
    
    # ========== 第二步：天气 API，获取温度和风速 ==========
    weather_url = "https://api.open-meteo.com/v1/forecast"
    weather_params = {
        "latitude": lat,
        "longitude": lon,
        "current": "temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m", # 你要的数据在这里
        "timezone": tz,
        "temperature_unit": "celsius"
    }
    
    weather_resp = requests.get(weather_url, params=weather_params, timeout=10)
    weather_data = weather_resp.json()
    
    # ========== 第三步：解析天气数据 ==========
    current = weather_data["current"]
    temp = current["temperature_2m"]        # 温度
    humidity = current["relative_humidity_2m"] # 湿度
    wind = current["wind_speed_10m"]        # 风速
    code = current["weather_code"]          # 天气代码
    desc = WMO_CODE_MAP.get(code, f"未知天气({code})")
    return desc
    #return f"{city}当前天气: {desc}, 温度 {temp}°C, 湿度 {humidity}%, 风速 {wind} km/h (天气代码: {code})"


def get_attraction(city: str, weather: str) -> str:
    api_key = "sk-45a486a23f96406eb8e70bab5ba3da18"
    deepseek = anthropic.Anthropic(api_key=api_key, base_url="https://api.deepseek.com/anthropic")
    query = f"推荐在{city}适合{weather}天气的旅游景点"
    try:
        response = deepseek.messages.create(
            max_tokens=1000,
            system="You are a helpful assistant.",
            model="deepseek-flash",
            messages = [{"role": "user", "content": query}],
            tools=[{"type": "web_search_20250305", "name": "web_search"}]
        )
        answer = "".join(block.text for block in response.content if block.type == "text")
        if not answer:
            return "No attractions found."
        return answer
    except Exception as e:
        return f"Error fetching attractions: {e}"

available_tools = {
        "get_weather": get_weather,
        "get_attraction": get_attraction
    }

class OpenAICompatibleClient():
    def __init__(self, model: str, api_key: str, base_url: str):
        self.model = model
        self.client = OpenAI(api_key=api_key, base_url=base_url)
    def generate(self, prompt: str, system_prompt: str) -> str:
        try:
            messages = [
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": prompt}
            ]
            response = self.client.chat.completions.create(
                model=self.model,
                messages=messages,
                stream=False
            )
            answer = response.choices[0].message.content
            print("response successfully received")
            return answer
        except Exception as e:
            print(f"Error generating response: {e}")
            return "LLM response generation failed. Please try again later."


LLM = OpenAICompatibleClient(
    model=MODEL,
    api_key=API_KEY,
    base_url=BASE_URL
)

user_prompt = "你好，请帮我查询⼀下今天北京的天⽓，然后根据天⽓推荐⼀个合适的旅游景点。"

prompt_history = [f"用户请求: {user_prompt}"]
print(f"⽤户输⼊: {user_prompt}\n" + "="*40)

if __name__ == "__main__":
    for i in range(5):
        print(f"第{i+1}轮对话:")
        full_prompt = "\n".join(prompt_history)
        llm_output = LLM.generate(prompt=full_prompt, system_prompt=AGENT_SYSTEM_PROMPT)
 
        # print(f"原始模型输出:\n{llm_output}\n")

        # match = re.search( r"Thought:\s*(.*?)\s*Action:\s*(.*)", llm_output, re.DOTALL)

        # if not match:
        #     observation = "Error: 模型输出没有遵循 Thought/Action 格式。"
        #     print(f"{observation}\n" + "=" * 40)
        #     prompt_history.append(f"Observation: {observation}")
        #     continue
        match = re.search(r'(Thought:.*?Action:.*?)(?=\n\s*(?:Thought:|Action:|Observation:)|\Z)', llm_output, re.DOTALL)
        if match:
            truncated = match.group(1).strip()
            if truncated != llm_output.strip():
                llm_output = truncated
                print("already truncated llm_output to match Thought-Action format")
            print(f"model output:\n{llm_output}\n")
            prompt_history.append(llm_output)
            action_match = re.search(r'Action:\s*(.*)', llm_output, re.DOTALL) 
            if not action_match:
                observation = "Error: No Action found in the model output."
                observation_str = f"Observation: {observation}"  
                print(f"{observation_str}\n" + "="*40) 
                prompt_history.append(observation_str)
                continue
            action_str = action_match.group(1).strip()
            if action_str.startswith("Finish"):
                final_answer = re.match(r"Finish\[(.*)\]", action_str).group(1)
                print(f"最终答案: {final_answer}\n" + "="*40)
                break

            tool_name = re.search(r"(\w+)\(", action_str).group(1)
            args_str = re.search(r"\((.*)\)", action_str).group(1)
            kwargs = dict(re.findall(r'(\w+)="([^"]*)"', args_str))

            if tool_name in available_tools:
                try:
                    observation = available_tools[tool_name](**kwargs)
                except Exception as e:
                    observation = f"工具执行失败: {e}"
            else:
                observation = f"错误：未定义的工具 '{tool_name}'"
            # if tool_name in available_tools:
            #     observation = available_tools[tool_name]
            # else:
            #     observation = f"错误:未定义的⼯具 '{tool_name}'"

            observation_str = f"Observation: {observation}"
            print(f"{observation_str}\n" + "="*40)
            prompt_history.append(observation_str)

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              