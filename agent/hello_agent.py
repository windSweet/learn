import requests
import os
from tavily import TavilyClient
from openai import OpenAI
import re

AGENT_SYSTEM_PROMPT ="""
你是⼀个智能旅⾏助⼿。你的任务是分析⽤户的请求，并使⽤可⽤⼯具⼀步步地解决问题。
# 可⽤⼯具:
- `get_weather(city: str)`: 查询指定城市的实时天⽓。
- `get_attraction(city: str, weather: str)`: 根据城市和天⽓搜索推荐的旅游景点。

# 输出格式要求:
你的每次回复必须严格遵循以下格式,包含⼀对Thought和Action:
Thought: [你的思考过程和下⼀步计划]
Action: [你要执⾏的具体⾏动]

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

API_KEY = "sk-aeWUFLG6keGrmYT50a129d4532564fB98dBe162eF95f0004"
BASE_URL = "https://aihubmix.com"
MODEL = "gpt-5.5-free"



def get_weather(city: str) -> str:
    url = f"https://wttr.in/{city}?format=j1"
    try: 
        response = requests.get(url)
        response.raise_for_status()
        data = response.json()
        weather_desc = data['current_condition'][0]['weatherDesc'][0]['value']
        return weather_desc
    except requests.RequestException as e:
        return f"Error fetching weather data: {e}"
    except (KeyError, IndexError) as e:
        return f"Error parsing weather data: {e}"


def get_attraction_tavily(city: str, weather: str) -> str:
    api_key = os.getenv("TAVILY_API_KEY")
    if not api_key:
        return "Error: TAVILY_API_KEY environment variable is not set."
    tavily = TavilyClient(api_key=api_key)
    query = f"推荐在{city}适合{weather}天气的旅游景点"
    try:
        response = tavily.search(query=query, search_depth="basic", include_answer=True)
        if response.get("answer"):
            return response["answer"]
        formatted_results = []
        for result in response.get("results", []):
            formatted_results.append(f"- {result.get('title', 'No Title')}: {result.get('snippet', 'No Snippet')}")
            if not formatted_results:
                return "No attractions found."
            return "sorry i cannt find any attraction for you"
        return "be based on the weather, here are some recommended attractions:\n" + "\n".join(formatted_results)
    except Exception as e:
        return f"Error fetching attractions: {e}"

def get_attraction(city: str, weather: str) -> str:
    api_key = "sk-0e3edcca34b2411fa0dc349b9a957662"
    deepseek = OpenAI(api_key=api_key, base_url="https://api.deepseek.com")
    query = f"推荐在{city}适合{weather}天气的旅游景点"
    try:
        response = deepseek.chat.completions.create(
            model="deepseek-flash",
            input = query,
            tools = [{"type": "web_search"}]
        )
        answer = response.output_text
        if not answer:
            return "No attractions found."
        return answer
    except Exception as e:
        return f"Error fetching attractions: {e}"


    available_tools = {
        "get_weather": get_weather,
        "get_attraction": get_attraction_tavily
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
            return response.choices[0].message.content
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

user_prompt = "我想去北京旅游，请问现在北京的天气如何？然后根据天气推荐一些旅游景点。"

prompt_history = [f"用户请求: {user_prompt}"]
print(f"⽤户输⼊: {user_prompt}\n" + "="*40)

if __name__ == "__main__":
    for i in range(5):
        print(f"第{i+1}轮对话:")
        full_prompt = "\n".join(prompt_history)
        llm_output = LLM.generate(prompt=full_prompt, system_prompt=AGENT_SYSTEM_PROMPT)
        match = re
