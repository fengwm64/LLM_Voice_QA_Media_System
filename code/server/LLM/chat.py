from zhipuai import ZhipuAI
import sys
client = ZhipuAI(api_key="d29ac61459b22352fe37d5f60c086387.HhnBAh9BCOAEyPAX") # 填写您自己的APIKey

#获取命令行参数
if len(sys.argv) < 2:
    print("Usage: python chatgml.py [your_prompt]")
    sys.exit(1)

prompt = sys.argv[1]   


response = client.chat.completions.create(
    model="glm-3-turbo",  # glm-3-turbo 填写需要调用的模型名称
    messages=[
        {"role": "user", "content": prompt},
    ],
    max_tokens=100,
)
print(response.choices[0].message)
#max_tokens=60, 每次返回的最大字数