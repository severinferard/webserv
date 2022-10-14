from urllib.request import Request
import requests
import socket
import pytest
from pathlib import Path
import subprocess

from common import *

s = requests.Session()

def test_POST_on_not_authorized_route_with_size_0():
    response = requests.post(BASE_URL + "/no-post", data="foo")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 405

def test_POST_on_not_authorized_route_with_size_10():
    response = requests.post(BASE_URL + "/no-post", data=("x" * 10))
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 405

def test_POST_on_not_authorized_route_with_size_1000():
    response = requests.post(BASE_URL + "/no-post", data=("x" * 1000))
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 405

def test_POST_without_content_length():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"POST / HTTP/1.1\r\n")
    conn.send(f"Host: {HOST}:{PORT}\r\n".encode())
    conn.send(f"User-Agent: curl/7.77.0\r\n".encode()) # No fieldname 
    conn.send(b"\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 411 Length Required"

def test_POST_with_content_length_too_short_check_status():
    headers = {}
    req = requests.Request(method='POST', url=BASE_URL + "/scripts/echo.php", headers=headers)
    prepped = req.prepare()
    prepped.headers["Content-Length"] = 10
    prepped.body = b'Seriously, send exactly these bytes.'
    print(prepped.headers)
    response = s.send(prepped)
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 200

def test_POST_with_content_length_too_short_check_value():
    headers = {}
    req = requests.Request(method='POST', url=BASE_URL + "/scripts/echo.php", headers=headers)
    prepped = req.prepare()
    prepped.headers["Content-Length"] = 10
    prepped.body = b'Seriously, send exactly these bytes.'
    print(prepped.headers)
    response = s.send(prepped)
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.content.decode() == "Seriously,"

@pytest.mark.skip(reason="Not Implemented")
def test_POST_with_content_length_too_long():
    pass

def gen_chunked_1():
    yield b'hi'
    yield b'there'

def test_POST_with_transfer_encoding_chunked_1_check_status():
    headers = {"Transfer-Encoding": "chunked"}
    response = requests.post(BASE_URL + "/scripts/echo.php", data=gen_chunked_1(), headers=headers)
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 200

def test_POST_with_transfer_encoding_chunked_1_check_value():
    headers = {"Transfer-Encoding": "chunked"}
    response = requests.post(BASE_URL + "/scripts/echo.php", data=gen_chunked_1(), headers=headers)
    pretty_print_request(response.request)
    pretty_print_response(response)
    desired_output = "".join([chunk.decode() for chunk in gen_chunked_1()])
    assert response.content.decode() == desired_output

def gen_chunked_2():
    for _ in range(8): # Cannot exceed 8388608 bytes for php-cgi so we send 8000000
        with open(WWW_DIR / 'onemillion.txt', "r") as f:
            yield f.read().encode()

def test_POST_with_transfer_encoding_chunked_2_check_status(create_1_million_file):
    headers = {"Transfer-Encoding": "chunked"}
    response = requests.post(BASE_URL + "/scripts/echo.php", data=gen_chunked_2(), headers=headers)
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 200

def test_POST_with_transfer_encoding_chunked_2_check_value(create_1_million_file):
    headers = {"Transfer-Encoding": "chunked"}
    response = requests.post(BASE_URL + "/scripts/echo.php", data=gen_chunked_2(), headers=headers)
    pretty_print_request(response.request)
    pretty_print_response(response)
    desired_output = "".join([chunk.decode() for chunk in gen_chunked_2()])
    assert response.content.decode() == desired_output

def gen_chunked_3():
    yield b'lolololololololololol'
    yield b'lalallalalalalalalalal'

def test_POST_with_transfer_encoding_chunked_body_limit_1():
    headers = {"Transfer-Encoding": "chunked"}
    response = requests.post(BASE_URL + "/post/limit", data=gen_chunked_3(), headers=headers)
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 413

def test_POST_with_transfer_encoding_chunked_body_limit_2(create_1_million_file):
    headers = {"Transfer-Encoding": "chunked"}
    try:
        response = requests.post(BASE_URL + "/post/limit", data=gen_chunked_2(), headers=headers)
        pretty_print_request(response.request)
        pretty_print_response(response)
        assert response.status_code == 413
    except requests.exceptions.ConnectionError: # Connection is closed by the server before the file has been fully sent
        pass
    except Exception:
        raise

def test_POST_body_limit_expect_413():
    try:
        response = requests.post(BASE_URL + "/post/limit", data=("a" * 1000))
        pretty_print_request(response.request)
        pretty_print_response(response)
        assert response.status_code == 413
    except requests.exceptions.ConnectionError: # Connection is closed by the server before the file has been fully sent
        pass
    except Exception:
        raise

def test_POST_body_limit_expect_200():
    response = requests.post(BASE_URL + "/post/limit", data=("a" * 10))
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 200

def test_POST_body_limit_1_million_expect_413(create_1_million_file):
    try:
        with open(WWW_DIR / 'onemillion.txt', "r") as f:
            response = requests.post(BASE_URL + "/post/limit", data=f.read())
        pretty_print_request(response.request)
        pretty_print_response(response)
        assert response.status_code == 413
    except requests.exceptions.ConnectionError: # Connection is closed by the server before the file has been fully sent
        pass
    except Exception:
        raise

def test_POST_body_limit_one_over_limit_expect_413():
    try:
        response = requests.post(BASE_URL + "/post/limit", data=("a" * 11))
        pretty_print_request(response.request)
        pretty_print_response(response)
        assert response.status_code == 413
    except requests.exceptions.ConnectionError: # Connection is closed by the server before the file has been fully sent
        pass
    except Exception:
        raise
    