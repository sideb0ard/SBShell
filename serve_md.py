# serve_md.py
import http.server, markdown, pathlib

class Handler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        path = pathlib.Path("." + self.path.split("?")[0])
        if path.suffix == ".md" and path.exists():
            body = markdown.markdown(path.read_text())
            html = f"<meta charset=utf-8><body>{body}</body>".encode()
            self.send_response(200)
            self.send_header("Content-Type", "text/html")
            self.end_headers()
            self.wfile.write(html)
        else:
            super().do_GET()

http.server.test(HandlerClass=Handler)
