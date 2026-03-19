git commit -m "Prompt changes" -- PROMPT.md
while true; do cat PROMPT.md | amp --mode deep 2>&1 | tee -a amp.log; done
