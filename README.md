# yarcb
Yet Another Rule-based ChatBot

This is a hobby project/pratice to make a simple, deterministic, rule-based chatbot without all the fancy stuff from SOTA LLMs,or other NLP.
Partly inspired by eliza (the ancient chatbot) and an obscure TUI RPG/SLG.

It aims to implement a variable list for more flexible rule matching. 

**DISCLAIMER**: I don't have a degree in CS, EE, or relevant fields, nor I am professional programmer, if anyone still reads the code, expect poor code quality, and/or (potential) bugs.(And watch out for buffer/memory issues)

### simple
Retro-style in C, no dependency, no fancy stuff.
### deterministic
Responses are solely determined by rules and state variables
### rule based
check below
### how to use
compile and run. Prepare a `./rule.conf'.
### rules
See example rule.conf (it's really unimaginative, but enough for an example)

the first section begins with [init](which should be the beginning line), and defines some named variables by $name=value.

All variables are strings.

Then after [rule] line there are the rules:

\<PATTERN\>|\<RESPONSE\>|\<WEIGHT\>(|\<CONDITION\>|\<ACTION\>)

\<CONDITION\>|\<ACTION\>are not implemented yet.

Each line contains a rule divided into five sections by delimiter '|'

\<PATTERN\> is a placeholder('-') separated string, and the input substrings that match the placeholders will be stored into $1-$9
(that being said, please use at most 9 '-'s)

eg. Matching "Alice was beginning to get very tired" against pattern "Alice was - to -" should store "beginning" and "get very tired" in $1 and $2.

\<RESPONSE\> is the response string for \<PATTERN\>, it can contain variables($1-$9, $a-$z) and will be replaced with their values before being printed.

\<WEIGHT\> is the rule's weight. If multiple rules' \<PATTERN\> match the input, the one with highest \<WEIGHT\> is selected.

Because the matching function return 0 (intuitive) when no match found, rule#0 should be the fallback.

#### predefined variables
$USER: Your name

$CHARA: Bot name

$SUF: suffix appended after each response

### Planned features:
custom conditons during matching and actions for the matched rule.

conditions may include: eq $var1 $var2,...

allow custom functions (in C code) to be called before/after match.

(eg. trim,lower,etc)

(Or allow "dummy rules" to trigger <ACTION> on <PATTERN> or <CONDITION>)

actions may include: set $var value,exit,add $var1 $var2/n (need numeric),...

define word lists in conf file and check if input contains words in the lists before matching rules, triggering corresponding flags or action.

add other predefined variables: $HISTn, $SUFFIX...

...

### note
punctuations or other special characters may be a problem.
