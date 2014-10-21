_fumanoid()
{
	local cur prev options switches commands
	COMPREPLY=()
	cur="${COMP_WORDS[COMP_CWORD]}"
	prev="${COMP_WORDS[COMP_CWORD-1]}"

	# collect options, switches and commands
	# parse help output of FUmanoids
	local section="none"
	while read line; do
		if [[ $line == Supported*options*  ]]; then section="options";  fi
		if [[ $line == Supported*switches* ]]; then section="switches"; fi
		if [[ $line == Supported*commands* ]]; then section="commands"; fi

		# split line on spaces
		local words=(${line// / })

		# the interesting lines start with letters, followed by spaces and a dash
		if [[ $line == [a-zA-Z]*\ -\ * ]]; then
			if [[ $section == "options"  ]]; then options="$options --${words[0]} "; fi
			if [[ $section == "switches" ]]; then switches="$switches --${words[0]} "; fi
			if [[ $section == "commands" ]]; then commands="$commands ${words[0]} "; fi
		fi
	done < <(${COMP_WORDS[0]} --help)

	# if any of the previous parameters was a command, fall back to file name matching
	# as after a command only custom parameters to that command can be issued and we have
	# no idea what they could be
	local param commandname
	for param in ${COMP_WORDS[@]}; do
		if [[ $param != -* ]]; then
			for commandname in $commands; do
				if [[ $param == $commandname ]]; then
					COMPREPLY=( $(compgen -f ${cur}) )
					return 0
				fi
			done
		fi
	done

	# if current word has an equal sign, we are in the value part (--key=value)
	if [[ ${cur} == *=* ]]; then
		# fall back to filename matching as we do not really know what to expect here
		COMPREPLY=( $(compgen -f ${cur}) )

	# if previous word was an option without equal sign, it requires an argument (--key value)
	elif [[ ${prev} == --* && ${prev} != *=* && $(compgen -W "${options}" -- ${prev}) ]]; then
		# fall back to filename matching as we do not know what the option expects
		COMPREPLY=( $(compgen -f ${cur}) )

	# if current word starts with a dash, it's either an option or a switch
	elif [[ ${cur} == -* ]]; then
		COMPREPLY=( $(compgen -W "${options} ${switches}" -- ${cur}) )

	# if it's not an option, it should be a command
	else
		COMPREPLY=( $(compgen -W "${commands}" -- ${cur}) )
	fi

	return 0
}

complete -F _fumanoid FUmanoid

