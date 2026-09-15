import requests
import os
from tavily import TavilyClient
from openai import OpenAI
import re
import anthropic

from urllib.parse import quote



import requests


#print(get_weather("北京"))
    # try: 
    #     response = requests.get(url)
    #     response.raise_for_status()
    #     data = response.json()
    #     weather_desc = data['current_condition'][0]['weatherDesc'][0]['value']
    #     return weather_desc
    # except requests.RequestException as e:
    #     return f"Error fetching weather data: {e}"
    # except (KeyError, IndexError) as e:
    #     return f"Error parsing weather data: {e}"



