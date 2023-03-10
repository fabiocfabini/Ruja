#!/bin/bash

./exe/ruja input.ruja > g.dot && dot -Tpdf g.dot -o g.pdf && xdg-open g.pdf