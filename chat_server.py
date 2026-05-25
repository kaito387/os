#!/usr/bin/env python3
"""
OS Study Chat - A chat interface for asking OS homework questions to DeepSeek V4 Flash.
Supports streaming, thinking mode, and a clean web UI.

Usage:
    python3 chat_server.py
    Then open http://localhost:5000 in your browser.
"""

import os
import json
import time
from flask import Flask, request, Response, render_template, stream_with_context
from openai import OpenAI

app = Flask(__name__)

# ---------- Configuration ----------
DEEPSEEK_API_KEY = os.environ.get("DEEPSEEK_API_KEY", "")
MODEL = "deepseek-v4-flash"
SYSTEM_PROMPT = (
    "You are a knowledgeable teaching assistant for an Operating Systems course. "
    "You help students understand OS concepts like processes, threads, synchronization, "
    "memory management, file systems, and I/O. "
    "Provide clear, detailed explanations with examples when appropriate. "
    "When answering programming questions, show concise code snippets. "
    "Be encouraging and pedagogical — explain not just the what, but also the why."
)

# ---------- DeepSeek Client (bypass system proxy) ----------
# Unset proxy env vars that may conflict with httpx
for _key in ("HTTP_PROXY", "HTTPS_PROXY", "http_proxy", "https_proxy", "ALL_PROXY", "all_proxy"):
    os.environ.pop(_key, None)

client = OpenAI(
    api_key=DEEPSEEK_API_KEY,
    base_url="https://api.deepseek.com",
)


def stream_chat(messages: list[dict]):
    """Generator that yields SSE events from the DeepSeek streaming API."""
    try:
        response = client.chat.completions.create(
            model=MODEL,
            messages=messages,
            stream=True,
            extra_body={"thinking": {"type": "enabled"}},
        )

        for chunk in response:
            delta = chunk.choices[0].delta if chunk.choices else None
            if delta is None:
                continue

            # Reasoning / thinking content
            if hasattr(delta, "reasoning_content") and delta.reasoning_content:
                yield f"data: {json.dumps({'type': 'thinking', 'content': delta.reasoning_content})}\n\n"

            # Regular content
            if delta.content:
                yield f"data: {json.dumps({'type': 'content', 'content': delta.content})}\n\n"

        # Signal completion
        yield f"data: {json.dumps({'type': 'done'})}\n\n"

    except Exception as e:
        yield f"data: {json.dumps({'type': 'error', 'content': str(e)})}\n\n"


# ---------- Routes ----------
@app.route("/")
def index():
    return render_template("index.html")


@app.route("/chat", methods=["POST"])
def chat():
    data = request.get_json()
    user_message = data.get("message", "").strip()
    history = data.get("history", [])

    if not user_message:
        return {"error": "Empty message"}, 400

    # Build messages array
    messages = [{"role": "system", "content": SYSTEM_PROMPT}]
    for msg in history:
        messages.append({"role": msg["role"], "content": msg["content"]})
    messages.append({"role": "user", "content": user_message})

    return Response(
        stream_with_context(stream_chat(messages)),
        content_type="text/event-stream",
        headers={
            "Cache-Control": "no-cache",
            "Connection": "keep-alive",
            "X-Accel-Buffering": "no",
        },
    )


@app.route("/health")
def health():
    return {"status": "ok", "model": MODEL, "has_key": bool(DEEPSEEK_API_KEY)}


# ---------- Main ----------
if __name__ == "__main__":
    if not DEEPSEEK_API_KEY:
        print("\n⚠️  WARNING: DEEPSEEK_API_KEY environment variable is not set!")
        print("   Set it with: export DEEPSEEK_API_KEY='your-key-here'\n")

    print(f"🚀 OS Study Chat starting...")
    print(f"   Model: {MODEL}")
    print(f"   Open http://localhost:5000 in your browser\n")
    app.run(host="0.0.0.0", port=5000, debug=True)
