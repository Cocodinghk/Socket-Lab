#!/Users/lizian/opt/anaconda3/bin/python
from os import environ
import json
import time
import os
from cv2 import recoverPose


log_file_path = "./cgi-bin/cgi_logPY.txt"
template_html_path = "./static_site/cgi_html/template.html"

f = open(log_file_path, "a")
sheet = input()
sheet = eval(sheet)


print("successfully got here!", file=f)

template_html_file = open(template_html_path, "r")


template_html = template_html_file.read()
template_html = template_html.replace("YourName", sheet['name'])
template_html = template_html.replace("YourPassword", sheet['password'])

response_content = ""


response_content += environ['SERVER_PROTOCOL']

response_content += " 200 OK\r\n"




response_content += "Server: "
response_content += environ['SERVER_SOFTWARE']
response_content += "\r\n"


response_content += "Date: "
response_content += time.strftime("%a, %d %b %Y %H:%M:%S GMT", time.gmtime())
response_content += "\r\n"

response_content += "Content-Length: "
response_content += str(len(template_html))
response_content += "\r\n"

response_content += "Content-Type: text/html"
response_content += "\r\n"

response_content += "Last-Modified: "
response_content += time.strftime("%a, %d %b %Y %H:%M:%S GMT", time.gmtime(os.path.getmtime(log_file_path)))
response_content += "\r\n"


response_content += "Connection: "
# response_content += environ['HTTP_CONNECTION']
response_content += "Keep-alive"



response_content += "\r\n\r\n"

response_content += template_html

print(response_content, file=f)
print(response_content)


