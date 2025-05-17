# r2ai.c Repository Guide

## Overview
This repository contains the C implementation of the r2ai plugin for radare2. Key source files include:
- `r2ai.c` – main plugin commands and LLM call dispatcher.
- `openai.c` / `anthropic.c` – wrappers for specific LLM providers.
- `r2ai_http.c` – HTTP helper with retry and timeout logic.
- `messages.c` – conversation and message management APIs.
- `tools.c` – tool execution helpers and tool JSON handling.
- `auto.c` – auto mode conversation loop.
- `markdown.c` – Markdown rendering for terminal output.
- `vdb.c` and `vdb_embed.inc.c` – simple vector database used for embeddings.

There were no tests or memory-bank directory initially. The Makefile builds a radare2 plugin.

## Tips
- Always consult `AGENTS.md` in repo root for mandatory steps (listing system binaries, updating memory bank, etc.).
- Run `find . -maxdepth 2 -type f` to get an overview of files.
- Functions use Radare2 APIs extensively; reading radare2 headers may help.
- Use `clang-format` for code style. Scripts `indent.py` and `indent2.py` help with formatting.
- The code uses PJ (printf-style JSON builder) from r2 libs.

## Testing
- Run `make test` to compile and run unit tests in the `tests/` directory.
- Tests link against radare2 core libraries using pkg-config.
- `r_json_parse` modifies its input buffer, so pass mutable strings (use `strdup`) in tests.
- Interactive editors like `vi` and `nano` are not installed; rely on `sed` or
  other command-line tools for editing files.
