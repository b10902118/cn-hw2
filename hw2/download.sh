curl --output down -X GET \
  'localhost:9999/api/file/b.mp4' \
  -H 'Connection: keep-alive' \
  -H 'Content-Type: multipart/form-data' \
  -H 'Authorization: Basic dXNlcm5hbWUxOnBhc3N3b3JkMQ==' \
  -H 'Cache-Control: no-cache' \
  -H 'Upgrade-Insecure-Requests: 1' \
  -H 'User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko)' \
  -H 'Accept: text/html, application/xhtml+xml, application/xml;q=0.9, image/avif, image/webp, image/apng' \
  -H 'Content-Length: 0' \
  -H 'Accept-Language: zh-TW, zh;q=0.9, en-US;q=0.8, en;q=0.7, fr;q=0.6, ms;q=0.5, zh-CN;q=0.4'

