/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: grenaud- <grenaud-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/07 14:04:58 by grenaud-          #+#    #+#             */
/*   Updated: 2023/02/21 16:57:40 by grenaud-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#define ERR_CD "error: cd: cannot change directory to "
#define ERR_ARG "error: cd: bad arguments"
#define ERR_EXC "error: cannot execute "
#define ERR_FTL "error: fatal"

void	ft_putchar(char s)
{
	write(STDERR_FILENO, &s, 1);
}

void	ft_putstr(char *s)
{
	int i = 0;
	while (s[i])
		ft_putchar(s[i++]);
}

void	errorminator(char *s, char *argv)
{
	ft_putstr(s);
	if (argv)
		ft_putstr(argv);
	ft_putchar('\n');
}

int	is_pipe(char **argv)
{
	int i = 0;
	int ret = 0;

	while(argv[i])
	{
		if (!strncmp(argv[i], "|", 2))
		{
			ret = 1;
			argv[i] = NULL;
			break;
		}
		if (!strncmp(argv[i], ";", 2))
		{
			ret = 0;
			argv[i] = NULL;
			break;
		}
		i++;
	}
	return (ret);
}

void	b_cd(char **argv)
{
	int i = 0;

	while (argv[i])
		i++;
	if (i != 2)
		errorminator(ERR_ARG, NULL);
	else if (chdir(argv[1]) < 0)
		errorminator(ERR_CD, argv[1]);
}

void	cmd(int argc, char **argv, char **envp, int prev, int *fd_prev)
{
	int		exit_status;
	int		i;
	int		next;
	int		fd_next[2];
	pid_t	pid;

	if (argc < 2)
		return ;
	next = is_pipe(argv);
	if (argv[0])
	{
		if (next && pipe(fd_next) < 0)
		{
			errorminator(ERR_FTL, NULL);
			exit (EXIT_FAILURE);
		}
		if (!strncmp(argv[0], "cd", 3))
			b_cd(argv);
		else if ((pid = fork()) < 0)
		{
			errorminator(ERR_FTL, NULL);
			exit (EXIT_FAILURE);
		}
		else if (pid == 0)
		{
			if (prev && dup2(fd_prev[0], 0) < 0)
				exit (-1);
			if (next && dup2(fd_next[1], 1) < 0)
				exit (-1);
			if (execve(argv[0], argv, envp) < 0)
			{
				errorminator(ERR_EXC, argv[0]);
				exit (EXIT_FAILURE);
			}
		}
		else
		{
			if (waitpid(pid, &exit_status, 0) < 0)
			{
				errorminator(ERR_FTL, NULL);
				exit (EXIT_FAILURE);
			}
			if (prev)
				close(fd_prev[0]);
			if (next)
				close(fd_next[1]);
		}
	}
	i = 0;
	while (argv[i])
		i++;
	cmd(argc - 1 - i, argv + 1 + i, envp, next, fd_next);
}

int		main(int argc, char **argv, char **envp)
{
	int	prev;
	int	fd_prev[2];

	prev = 0;
	fd_prev[0] = 0;
	fd_prev[1] = 1;
	cmd(argc, argv + 1, envp, prev, fd_prev);
	exit (EXIT_SUCCESS);
}