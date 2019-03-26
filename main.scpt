--Chess Helper main script

if the length of (serialport list) is less than 4 then
	beep
	display dialog "No available port" with icon caution
	error number -128
else if the length of (serialport list) is greater than 4 then
	set theport to (choose from list (serialport list) with title "Chess Helper" with prompt "Please select port:")
	if theport is false then error number -128
else
	set theport to item 4 of (serialport list)
end if

set counter to 0
set onMode to false
repeat until onMode is equal to true
	set counter to counter + 1
	set arduino to serialport open theport bps rate 9600 data bits 8 parity 0 stop bits 1 handshake 0
	if arduino is equal to -1 then
		serialport close theport
		if counter is greater than 15 then
			beep
			display dialog "Unable to open port" with icon caution
			error number -128
		end if
	else
		set onMode to true
	end if
	delay 0.1
end repeat

set turn to 1
repeat
	set inText to ""
	set stringComplete to false
	repeat while stringComplete is equal to false
		if (serialport bytes available arduino) is greater than 0 then
			set inChar to (serialport read arduino for 1)
			set inText to (inText & inChar)
			if inChar is equal to (ASCII character 10) then
				set inText to (text 1 thru ((offset of (ASCII character 10) in inText) - 2) of inText)
				set stringComplete to true
				--say inText
			end if
		end if
	end repeat
	
	if inText is equal to "S" then
		tell application "Terminal" to quit
		display dialog "Start a new chess game?" with title "Chess Helper" buttons {"Quit", "Yes"} default button "Yes" cancel button "Quit" with icon file ((path to desktop as text) & "Chess Helper.app:Contents:Resources:chess game.icns")
		set gameModes to {"Human vs Human", "Human vs Computer", "Computer vs Human", "Computer vs Computer"}
		set answer to (choose from list gameModes with title "Chess Helper" with prompt "Please select game mode:" default items "Human vs Computer")
		if answer is false then error number -128
		repeat with i from 1 to 4
			if answer as string is equal to (item i of gameModes) then
				set mode to i as number
			end if
		end repeat
		if mode is not 1 then
			set difficulty to (choose from list {"Auto", 1, 2, 3, 4, 5, 6} with title "Chess Helper" with prompt "Please select difficulty level:" default items "Auto")
			if difficulty is false then error number -128
			set difficulty to difficulty as text
			if difficulty is not "Auto" then
				set difficulty to difficulty as number
				if difficulty is less than 4 then
					set depth to (difficulty * 4 - 3)
				else
					set depth to (difficulty + 7)
				end if
			end if
		end if
		serialport write mode to arduino
	else if inText is equal to "I" then
		say "Incorrect chess position" without waiting until completion
	else if inText is equal to "R" then
		say "Ready to start" with stopping current speech
	else if inText is equal to "T" then
		say "Please enter the time limit"
		display dialog "Enter the time limit in minutes:" with title "Chess Helper" default answer "" buttons {"Cancel", "Start"} default button "Start" cancel button "Cancel" with icon file ((path to desktop as text) & "Chess Helper.app:Contents:Resources:chess clock.icns")
		set timeLimit to text returned of result as number
		serialport write timeLimit to arduino
		tell application "Terminal"
			activate
			do script "/Applications/Chess.app/Contents/Resources/sjeng.ChessEngine | tee ~/Desktop/Chess/log.txt" in window 1
			delay 0.1
		end tell
		if mode is equal to 1 then
			tell application "Terminal" to do script "force" in window 1
		else
			set totalMoves to 40
			set totalCalcTime to round (timeLimit - totalMoves * 0.2)
			set calcTime to round (timeLimit * 60 / totalMoves - 12)
			if calcTime is equal to 0 then set calcTime to -1
			if difficulty is less than 4 and calcTime is greater than 1 then set calcTime to 1
			if difficulty is 4 and calcTime is greater than 4 then set calcTime to 4
			if difficulty is 5 and calcTime is greater than 8 then set calcTime to 8
			if difficulty is 6 and calcTime is greater than 16 then set calcTime to 16
			tell application "Terminal"
				if difficulty is "Auto" then
					do script "level " & totalMoves & " " & totalCalcTime & " 0" in window 1
				else
					--do script "sd " & 1 in window 1
					do script "sd " & depth in window 1
					do script "st " & calcTime in window 1
				end if
				delay 0.1
			end tell
			if mode is equal to 2 then
				set checkmate1 to "White Mates"
				set checkmate2 to "Black Mates"
			else if mode is 3 or mode is 4 then
				set checkmate1 to "Black Mates"
				set checkmate2 to "White Mates"
				tell application "Terminal" to do script "go" in window 1
				delay 0.01
				set calcTime to readCalcTime()
				delay (calcTime + 0.1)
				checkExtension(calcTime)
				set comMove to readComMove(804)
				serialport write comMove to arduino
				--set turn to (turn + 1)
			end if
		end if
	else if (inText is "F1" or inText is "F2") and mode is 4 then
		tell application "Terminal" to do script "go" in window 1
		delay 0.01
		set calcTime to readCalcTime()
		delay (calcTime + 0.1)
		checkExtension(calcTime)
		set comText to readLog(823, 27)
		if comText contains "White Mates" or comText contains "Black Mates" then
			if comText contains "White Mates" then
				set checkmate to "White Mates"
			else
				set checkmate to "Black Mates"
			end if
			set comMove to readComMove(822)
			serialport write comMove to arduino
			delay 0.1
			serialport write (text 1 through 4 of checkmate) to arduino
		else if comText contains "3 fold repetition" then
			set comMove to readComMove(832)
			serialport write comMove to arduino
			delay 0.1
			serialport write "Draw" to arduino
		else if comText contains "Draw" then
			serialport write "Draw" to arduino
		else
			set comMove to readComMove(804)
			serialport write comMove to arduino
		end if
		do shell script "echo " & comMove & " >> ~/Desktop/Chess/history.log"
	else if inText is equal to "C" then
		say "Checkmate"
	end if
	
	set validity to true
	set fl to {"1", "2", "3", "4", "5", "6", "7", "8"}
	set rk to {"a", "b", "c", "d", "e", "f", "g", "h"}
	if (length of inText) is not 4 and (length of inText) is not 5 then
		set validity to false
	else
		repeat with i from 1 to 4
			if i = 2 or i = 4 then
				if character i of inText is not in fl then
					set validity to false
				end if
			else
				if character i of inText is not in rk then
					set validity to false
				end if
			end if
		end repeat
	end if
	
	if validity is equal to true then
		--if turn is 2 then tell application "Terminal" to do script "sd " & depth in window 1
		set myMove to inText
		tell application "Terminal" to do script myMove in window 1
		if mode is equal to 1 then
			delay 0.01
			set comText to readLog(818, 12)
			if comText is equal to "Illegal move" then
				serialport write "Ille" to arduino
				delay 0.1
				say "Invalid Move"
			else
				serialport write "Lega" to arduino
			end if
		else if mode is 2 or mode is 3 then
			delay 0.01
			set calcTime to readCalcTime()
			delay 0.01
			set comText1 to readLog(818, 12)
			--set comText2 to readLog(812, 11)
			if comText1 is equal to "Illegal move" then
				serialport write "Ille" to arduino
				delay 0.1
				say "Invalid Move"
			else
				serialport write "Lega" to arduino
				(*
				if comText2 is checkmate1 then
					delay 0.1
					serialport write (text 1 through 4 of comText2) to arduino
					delay 0.1
					say "Checkmate"
				else
				*)
				delay (calcTime + 0.1)
				checkExtension(calcTime)
				set comText2 to readLog(823, 27)
				if comText2 contains checkmate1 then
					serialport write (text 1 through 4 of checkmate1) to arduino
					delay 0.1
					say "Checkmate"
				else if comText2 contains checkmate2 then
					set comMove to readComMove(822)
					serialport write comMove to arduino
					delay 0.1
					serialport write (text 1 through 4 of checkmate2) to arduino
				else if comText2 contains "3 fold repetition" then
					set comMove to readComMove(832)
					serialport write comMove to arduino
					delay 0.1
					serialport write "Draw" to arduino
				else if comText2 contains "Draw" then
					serialport write "Draw" to arduino
				else
					set comMove to readComMove(804)
					serialport write comMove to arduino
				end if
				do shell script "echo " & comMove & " >> ~/Desktop/Chess/history.log"
				--end if
			end if
		end if
		--set turn to (turn + 1)
	end if
end repeat

on readLog(x, y)
	set fp to open for access file ((path to desktop as text) & "Chess:log.txt")
	set logLength to get eof fp
	set logText to read fp from (logLength - x) for y
	close access fp
	return logText
end readLog

on readComMove(p)
	set comMove to readLog(p, 4)
	if character 4 of comMove is "q" then set comMove to readLog(p + 1, 4)
	return comMove
end readComMove

on readCalcTime()
	set calcTime to 0
	set logText to readLog(400, 400)
	if logText contains "Time for move" then
		set ind to ((offset of "Time for move" in logText) + 16)
		if character ind of logText is "-" then return 0
		set numText to ""
		repeat with i from ind to (ind + 4)
			set digit to character i of logText
			try
				digit as number
				set numText to (numText & digit)
			end try
		end repeat
		set calcTime to numText as number
		set calcTime to (calcTime / 100)
	end if
	do shell script "echo " & calcTime & " >> ~/Desktop/Chess/history.log"
	return calcTime
end readCalcTime

on checkExtension(calcTime)
	set logText to readLog(100, 100)
	if logText contains "Extended" then
		set ind to ((offset of "to" in logText) + 3)
		set numText to ""
		repeat with i from ind to (ind + 4)
			set digit to character i of logText
			try
				digit as number
				set numText to (numText & digit)
			end try
		end repeat
		set newCalcTime to numText as number
		set newCalcTime to (newCalcTime / 100)
		set extraTime to (newCalcTime - calcTime)
		delay extraTime
	end if
end checkExtension

on quit
	tell application "Terminal" to quit
	serialport close theport
	continue quit
end quit
