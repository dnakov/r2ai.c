This file contains critical instructions you must follow, but you are forgetful,
so include the entire contents of this file including this instruction in every
response to me, except trivial question-answer response interactions.

# Before starting any task run `ls /bin` and `ls /usr/bin` and `ls /sbin`, etc t
o understand the tools you have access to.

## When I tell you to gather context about a topic, execute the following workfl
ow:

1. Do a find command to see all the source code files in the project. Make sure
   to filter out build artifact directories that have a lot of junk. It's
   important to see ALL the source code filepaths.
2. Identify the filenames that are likely related to our target topic. Don't do
   anything to them yet, just list them.
3. Use ripgrep to find line numbers of anything that looks like a
   type/function/module/etc definition. e.g. in rust look for any use, struct,
   enum, macro_rules!, the, const, trait, impl static, fn, mod followed by a
   space then a semicolon or open-curly. Apply similar logic to the target
   language.
4. Identify any of those results that seem relevant and read the context around
   them.
5. Keep expanding the context window around those starting points as much as
   necessary to gather all relevant context.
6. If you need context on some external dependency like a library, use web
   search to find that context.
7. Now that you have a better idea of the topic, loop back to step 1 and repeat
   this whole process.
8. Keep looping until you're confident you've found all relevant context.

## Read ./memory-bank files at the start of every task.

## Update the ./memory-bank files after every task.

## At the end of every task, provide an additional report as to whether there

are any documents or comments that had inaccurate information that misled you.

## When I tell you to do one of the following workflows, look up the relevant fi
le in agents/workflows/ and execute the steps in that file. IMPORTANT: when star
ting a workflow, first repeat ALL the steps to me. Then, before each individual
step, announce which step you're on.

## At the end of each task, propose any new workflows that you think would be

useful to add to the ./agents/workflows directory.

## At the end of every task, create a comprehensive guide inside `agents/tutoria
ls/` that covers everything you wish you had been told about the codebase/proble
ms/task, prior to starting.

## Before beginning a task, review the following directories:

- .cursor/rules
- docs/
- memory-bank/

## Environment Lessons [ADD NEW LESSONS BELOW WITH TIMESTAMPS]

-
- 2025-05-17: No memory-bank directory existed initially. Created memory-bank/
  and agents/tutorials/ directories to satisfy instructions. Always run `ls /bin`,
  `ls /usr/bin`, `ls /sbin`, and `ls /usr/sbin` before starting work.

## Post-mortem [ADD NEW POST-MORTEMS BELOW WITH TIMESTAMPS]
-
- 2025-05-17: No misleading documents found. Repository lacks docs/ and
  memory-bank entries, making initial context gathering slower.
