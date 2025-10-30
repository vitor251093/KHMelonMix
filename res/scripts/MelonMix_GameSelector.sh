#!/bin/bash
function Start {
	while true; do
		echo -e "\033]0;MelonMix - Select a game\007"
		game=""
		echo "Which game would you like to play?"
		echo "1) Kingdom Hearts 358/2 Days"
		echo "2) Kingdom Hearts Re:Coded"
		echo "3) Quit"
		read game
		case "${game,,}" in
			"1" | "kingdom hearts 358/2 days" | "358/2 days" | "days")
				gamename="Kingdom Hearts 358/2 Days"
				game="days"
				break
			;;
			"2" | "kingdom hearts re:coded" | "kingdom hearts recoded" | "re:coded" | "recoded" | "coded")
				gamename="Kingdom Hearts Re:Coded"
				game="recoded"
				break
			;;
			"3" | "n" | "no" | "q" | "quit" | "c" | "close" | "cancel" | "exit" | "x" | "end")
				exit 0
			;;
			*)
				echo "${game}" is not a valid answer
			;;
		esac
	done
}

function Fullscreen {
	while true; do
		echo -e "\033]0;MelonMix - ${gamename}\007"
		echo
		toggle="1"
		echo "${gamename}:"
		echo "1) Fullscreen"
		echo "2) Windowed"
		echo "3) Back"
		echo "4) Quit"
		read toggle
		case "${toggle,,}" in
			"1" | "fullscreen")
				fullscreen="-f"
				break
			;;
			"2" | "windowed")
				fullscreen=""
				break
			;;
			"3" | "b" | "back" | "n" | "no" | "cancel")
				Start
			;;
			"4" | "q" | "quit" | "c" | "close" | "exit" | "x" | "end")
				exit 0
			;;
			*)
				echo "${toggle}" is not a valid answer
			;;
		esac
	done
}

function Game {
	while true; do
		echo
		if [[ -f roms/${game}.nds ]]; then
			echo "Starting ${gamename}"
			./MelonMix ${fullscreen} roms/${game}.nds
			exit 0
		else
			echo "Error: ${game}.nds was not found in roms folder..."
			echo
			Start
			Fullscreen
		fi
	done
}

Start
Fullscreen
Game