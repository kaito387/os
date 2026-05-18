#!/usr/bin/env python3
"""
AnkiConnect helper — wraps the AnkiConnect JSON-RPC API for creating flashcards.
Usage:
    python3 ankiconnect.py health
    python3 ankiconnect.py create-model
    python3 ankiconnect.py check-deck <deck>
    python3 ankiconnect.py add-note <deck> <model> <csv-file>   # single note from CSV row (pipe in)
    python3 ankiconnect.py add-notes <deck> <model> <csv-file>  # batch import
"""

import sys
import json
import urllib.request

ANKICONNECT_URL = "http://localhost:8765"

def _rpc(action, params=None):
    """Send a JSON-RPC 2.0 request to AnkiConnect."""
    payload = {
        "action": action,
        "version": 6,
    }
    if params is not None:
        payload["params"] = params

    req = urllib.request.Request(
        ANKICONNECT_URL,
        data=json.dumps(payload).encode("utf-8"),
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    try:
        with urllib.request.urlopen(req, timeout=5) as resp:
            result = json.loads(resp.read().decode("utf-8"))
        if result.get("error") is not None:
            print(f"Error: {result['error']}", file=sys.stderr)
            return None
        return result.get("result")
    except urllib.error.URLError as e:
        print(f"Cannot reach AnkiConnect at {ANKICONNECT_URL}: {e}", file=sys.stderr)
        return None
    except Exception as e:
        print(f"Unexpected error: {e}", file=sys.stderr)
        return None


def cmd_health():
    """Check if AnkiConnect is reachable."""
    result = _rpc("version")
    if result is not None:
        print(f"AnkiConnect reachable (v{result})")
        return True
    return False


def cmd_check_deck(deck_name):
    """Check if a deck exists; create it if not."""
    decks = _rpc("deckNames")
    if decks is None:
        return False
    if deck_name not in decks:
        print(f"Creating deck: {deck_name}")
        _rpc("createDeck", {"deck": deck_name})
    return True


def cmd_create_model():
    """Create the OS-Concept note type with 6 fields and 2 card templates."""
    model_name = "OS-Concept"
    model = {
        "modelName": model_name,
        "inOrderFields": ["Question", "Topic", "Subtopic", "Answer", "Example", "Notes"],
        "css": """
.topic {
  font-weight: bold;
  color: #2E86AB;
  font-size: 14px;
  margin-bottom: 6px;
}
.question {
  font-size: 18px;
  font-weight: bold;
  color: #333;
}
.answer {
  font-size: 16px;
  color: #111;
  line-height: 1.6;
}
.answer p {
  margin: 0 0 8px 0;
}
.answer ul, .example ul, .notes ul {
  margin: 4px 0 8px 0;
  padding-left: 20px;
}
.answer li, .example li, .notes li {
  margin-bottom: 4px;
  list-style-type: disc;
}
.answer code, .example code, .notes code {
  background: #f0f0f0;
  padding: 1px 4px;
  border-radius: 3px;
  font-family: monospace;
  font-size: 0.9em;
}
.answer pre {
  background: #f5f5f5;
  padding: 8px;
  border-radius: 4px;
  overflow-x: auto;
}
.example {
  font-size: 15px;
  color: #555;
  line-height: 1.5;
}
.example p {
  margin: 0 0 6px 0;
}
.notes {
  font-size: 14px;
  color: #888;
  font-style: italic;
  line-height: 1.5;
}
.notes p {
  margin: 0 0 4px 0;
}
hr {
  border: 0;
  border-top: 1px solid #ccc;
  margin: 8px 0;
}
""",
        "cardTemplates": [
            {
                "Name": "Concept → Answer",
                "Front": (
                    '<div class="topic">{{Topic}} / {{Subtopic}}</div>\n'
                    '<hr>\n'
                    '<div class="question">{{Question}}</div>'
                ),
                "Back": (
                    '<div class="topic">{{Topic}} / {{Subtopic}}</div>\n'
                    '<hr>\n'
                    '<script type="text/plain" id="answer-raw">{{Answer}}</script>\n'
                    '<div class="answer" id="answer-field"></div>\n'
                    '{{#Example}}<hr>\n'
                    '<script type="text/plain" id="example-raw">{{Example}}</script>\n'
                    '<div class="example" id="example-field"></div>\n'
                    '{{/Example}}\n'
                    '{{#Notes}}<hr>\n'
                    '<script type="text/plain" id="notes-raw">{{Notes}}</script>\n'
                    '<div class="notes" id="notes-field"></div>\n'
                    '{{/Notes}}\n'
                    '<script>\n'
                    '(function() {\n'
                    '  function inlineFmt(text) {\n'
                    '    // bold: **text** or __text__\n'
                    '    text = text.replace(/\\*\\*(.+?)\\*\\*/g, "<b>$1</b>");\n'
                    '    text = text.replace(/__(.+?)__/g, "<b>$1</b>");\n'
                    '    // italic: *text* (but NOT **text**) — use negative lookbehind/lookahead\n'
                    '    text = text.replace(/(?<!\\*)\\*(?!\\*)(.+?)(?<!\\*)\\*(?!\\*)/g, "<i>$1</i>");\n'
                    '    text = text.replace(/\\b_(.+?)_\\b/g, "<i>$1</i>");\n'
                    '    // inline code: `text`\n'
                    '    text = text.replace(/`(.+?)`/g, "<code>$1</code>");\n'
                    '    return text;\n'
                    '  }\n'
                    '  function md(text) {\n'
                    '    if (!text || !text.trim()) return "";\n'
                    '    // Split into paragraphs (blank-line separated)\n'
                    '    var paragraphs = text.split(/\\n\\n+/);\n'
                    '    var result = paragraphs.map(function(p) {\n'
                    '      p = p.trim();\n'
                    '      if (!p) return "";\n'
                    '      var lines = p.split("\\n");\n'
                    '      // Check if every line starts with a bullet marker\n'
                    '      var isList = lines.length > 0 && lines.every(function(l) {\n'
                    '        return /^[•\\-]\\s/.test(l);\n'
                    '      });\n'
                    '      if (isList) {\n'
                    '        var items = lines.map(function(l) {\n'
                    '          var content = l.replace(/^[•\\-]\\s+/, "");\n'
                    '          return "<li>" + inlineFmt(content) + "</li>";\n'
                    '        });\n'
                    '        return "<ul>" + items.join("") + "</ul>";\n'
                    '      }\n'
                    '      // Regular paragraph: join lines with <br>\n'
                    '      return "<p>" + lines.map(inlineFmt).join("<br>") + "</p>";\n'
                    '    }).join("");\n'
                    '    // Clean up empty containers\n'
                    '    result = result.replace(/<p><\\/p>/g, "");\n'
                    '    result = result.replace(/<ul><\\/ul>/g, "");\n'
                    '    return result;\n'
                    '  }\n'
                    '  function render(id, label) {\n'
                    '    var raw = document.getElementById(id + "-raw");\n'
                    '    var display = document.getElementById(id + "-field");\n'
                    '    if (!raw || !display) return;\n'
                    '    var text = raw.textContent.trim();\n'
                    '    if (!text) return;\n'
                    '    var html = md(text);\n'
                    '    if (label && html) html = "<b>" + label + ":</b><br>" + html;\n'
                    '    display.innerHTML = html;\n'
                    '  }\n'
                    '  render("answer", "");\n'
                    '  render("example", "Example");\n'
                    '  render("notes", "Notes");\n'
                    '})();\n'
                    '</script>'
                ),
            },
        ],
    }

    # Check if model already exists
    existing = _rpc("modelNames")
    if existing and model_name in existing:
        print(f"Model '{model_name}' already exists — updating templates and CSS only.")
        print("NOTE: Field order cannot be updated via API.")
        print(f"If the first field is not 'Question', manually delete '{model_name}' in ANKI")
        print("(Tools → Manage Note Types → select the model → Delete), then re-run this command.")
        templates_dict = {t["Name"]: {"Front": t["Front"], "Back": t["Back"]} for t in model["cardTemplates"]}
        # Also include an empty placeholder to neutralize any stale old templates (e.g., from v1)
        templates_dict["Example → Concept"] = {"Front": "", "Back": ""}
        _rpc("updateModelTemplates", {"model": {"name": model_name, "templates": templates_dict}})
        _rpc("updateModelStyling", {"model": {"name": model_name, "css": model["css"]}})
    else:
        _rpc("createModel", model)
    print(f"Model '{model_name}' ready (6 fields, 2 card types)")
    return True


def cmd_add_note(deck, model, front, back, tags_str):
    """Add a single note to ANKI."""
    tags = [t.strip().lstrip("#") for t in tags_str.split(",") if t.strip()]
    params = {
        "note": {
            "deckName": deck,
            "modelName": model,
            "fields": {"Front": front, "Back": back},
            "tags": tags,
        }
    }
    result = _rpc("addNote", params)
    if result is not None:
        print(f"Created note: {result}")
        return True
    return False


def _parse_csv_row(row, model):
    """Map CSV row fields to the note model's fields."""
    if model == "OS-Concept":
        return {
            "Topic": row.get("Topic", ""),
            "Subtopic": row.get("Subtopic", ""),
            "Question": row.get("Question", ""),
            "Answer": row.get("Answer", ""),
            "Example": row.get("Example", ""),
            "Notes": row.get("Notes", ""),
        }
    else:
        # Legacy "Basic" model fallback
        return {
            "Front": row.get("Front", row.get("Question", "")),
            "Back": row.get("Back", row.get("Answer", "")),
        }


def cmd_add_notes(deck, model, csv_path):
    """Batch-add notes from a CSV file.
    CSV columns depend on model:
      - Basic:  Front,Back,Tags
      - OS-Concept: Topic,Subtopic,Question,Answer,Example,Notes,Tags
    """
    import csv
    notes = []
    with open(csv_path, "r", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            tags = [t.strip().lstrip("#") for t in row.get("Tags", "").split(",") if t.strip()]
            notes.append({
                "deckName": deck,
                "modelName": model,
                "fields": _parse_csv_row(row, model),
                "tags": tags,
            })

    results = _rpc("addNotes", {"notes": notes})
    if results is not None:
        success = sum(1 for r in results if r is not None)
        fails = sum(1 for r in results if r is None)
        print(f"Created {success} notes, {fails} duplicates/failures")
        return True
    return False


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    cmd = sys.argv[1]

    if cmd == "health":
        sys.exit(0 if cmd_health() else 1)

    elif cmd == "create-model":
        sys.exit(0 if cmd_create_model() else 1)

    elif cmd == "check-deck":
        if len(sys.argv) < 3:
            print("Usage: ankiconnect.py check-deck <deck>", file=sys.stderr)
            sys.exit(1)
        sys.exit(0 if cmd_check_deck(sys.argv[2]) else 1)

    elif cmd == "add-note":
        if len(sys.argv) < 5:
            print("Usage: ankiconnect.py add-note <deck> <model> <csv-row-json>", file=sys.stderr)
            sys.exit(1)
        # CSV row passed as JSON string for single-note creation
        row = json.loads(sys.argv[4])
        tags = [t.strip().lstrip("#") for t in sys.argv[5].split(",")] if len(sys.argv) > 5 else []
        note = {
            "deckName": sys.argv[2],
            "modelName": sys.argv[3],
            "fields": _parse_csv_row(row, sys.argv[3]),
            "tags": tags,
        }
        result = _rpc("addNote", {"note": note})
        if result is not None:
            print(f"Created note: {result}")
            sys.exit(0)
        sys.exit(1)

    elif cmd == "add-notes":
        if len(sys.argv) < 4:
            print("Usage: ankiconnect.py add-notes <deck> <model> <csv-file>", file=sys.stderr)
            sys.exit(1)
        sys.exit(0 if cmd_add_notes(sys.argv[2], sys.argv[3], sys.argv[4]) else 1)

    else:
        print(f"Unknown command: {cmd}", file=sys.stderr)
        print(__doc__)
        sys.exit(1)


if __name__ == "__main__":
    main()
