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
}
.example {
  font-size: 15px;
  color: #555;
  font-family: monospace;
}
.notes {
  font-size: 14px;
  color: #888;
  font-style: italic;
}
hr {
  border: 0;
  border-top: 1px solid #ccc;
  margin: 6px 0;
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
                    '<div class="answer">{{Answer}}</div>\n'
                    '{{#Example}}<hr>\n<div class="example"><b>Example:</b><br>{{Example}}</div>{{/Example}}\n'
                    '{{#Notes}}<hr>\n<div class="notes"><b>Notes:</b><br>{{Notes}}</div>{{/Notes}}'
                ),
            },
            {
                "Name": "Example → Concept",
                "Front": (
                    '<div class="topic">{{Topic}} / {{Subtopic}}</div>\n'
                    '<hr>\n'
                    '<div class="example"><b>Example:</b><br>{{Example}}</div>\n'
                    '{{#Question}}<hr>\n<div class="question">{{Question}}</div>{{/Question}}'
                ),
                "Back": (
                    '<div class="topic">{{Topic}} / {{Subtopic}}</div>\n'
                    '<hr>\n'
                    '<div class="answer">{{Answer}}</div>\n'
                    '{{#Example}}<hr>\n<div class="example"><b>Recall:</b><br>{{Example}}</div>{{/Example}}\n'
                    '{{#Notes}}<hr>\n<div class="notes"><b>Notes:</b><br>{{Notes}}</div>{{/Notes}}'
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
        print("(Tools → Manage Note Types → select '{model_name}' → Delete), then re-run this command.")
        templates_dict = {t["Name"]: {"Front": t["Front"], "Back": t["Back"]} for t in model["cardTemplates"]}
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
