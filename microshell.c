/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell_testing_one_pipe.c                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aamorin- <aamorin-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/06 16:30:05 by aamorin-          #+#    #+#             */
/*   Updated: 2022/02/10 17:13:40 by aamorin-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pid_t pid;
int fd_in;
int fd[2];

typedef struct s_exe
{
	char	**c_split;
}	t_exe;

typedef struct s_pipe
{
	int		**pipes;
	int		*com_count;
	int		procecess_num;
	t_exe	*exe;
}	t_pipe;

int	ft_arraybilen(char **array)
{
	int	i;

	i = 0;
	while (array[i] != NULL)
		i++;
	return (i);
}

int	ft_strlen(char *str)
{
	int	i;

	i = 0;
	if (!str || str == NULL)
		return (0);
	while (str[i])
		i++;
	return (i);
}

int	ft_frlloc(char **tab)
{
	int	i;

	i = -1;
	while (tab[++i])
		free(tab[i]);
	free(tab);
	return (1);
}

t_pipe	init_pipex(int a, int i)
{
	t_pipe	pipex;

	pipex.procecess_num = a;
	pipex.exe = (t_exe *)malloc((pipex.procecess_num + 1) * sizeof(t_exe));
	pipex.com_count = (int *)malloc((pipex.procecess_num + 1) * sizeof(int));
	while (++i < pipex.procecess_num + 1)
	{
		pipex.com_count[i] = 0;
		pipex.exe[i].c_split = NULL;
	}
	return (pipex);
}

void	child(t_pipe pipex, char **envp, int i)
{
	dup2(fd_in, 0);
	if (i != pipex.procecess_num - 1)
		dup2(fd[1], 1);
	close(fd_in);
	close(fd[1]);
	close(fd[0]);
	int pid = fork();
	if (pid == -1)
	{
		write (2, "error: fatal\n", 13);
		exit(0);
	}
	else if (pid == 0)
	{
		if (!pipex.exe[i].c_split[0])
			exit (127);
		else if (execve(pipex.exe[i].c_split[0], pipex.exe[i].c_split, envp) == -1)
		{
			write (2, "error: cannot execute ", 22);
			write (2, pipex.exe[i].c_split[0], ft_strlen(pipex.exe[i].c_split[0]));
			write (2, "\n", 1);
			ft_frlloc(pipex.exe[i].c_split);
			exit (127);
		}
	}
	waitpid(0, NULL, 0);
	exit (0);
}

int ft_execv(char **argv, int i, char **envp)
{
    while (argv[++i])
    {
        if (strcmp(argv[i], ";"))
        {
            i--;
            break ;
        }
    }
	int		pipe_pos = 1;
	int		command_split_pos = 0;
	int		command_pos;
	int		index = i;

	while (argv[++i])
	{
		if (!strcmp(argv[i], "|"))
			pipe_pos++;
		else if (!strcmp(argv[i], ";") || ft_arraybilen(argv) < i)
			break ;
	}
	t_pipe pipex = init_pipex(pipe_pos, -1);
	i = index;
	pipe_pos = 0;
	while (argv[++i])
	{
		if (!strcmp(argv[i], "|"))
			pipe_pos++;
		else if (!strcmp(argv[i], ";" ) || ft_arraybilen(argv) < i)
			break ;
		else
			pipex.com_count[pipe_pos] += 1;
	}
	int a = -1;
	while (++a <= pipe_pos)
	{
		pipex.exe[a].c_split = (char **)malloc((pipex.com_count[a] + 1) * sizeof(char *));
		pipex.exe[a].c_split[pipex.com_count[a]] = NULL;
	}
	pipe_pos = 0;
	while (argv[++index])
	{
		if (!strcmp(argv[index], "|"))
		{	
			pipe_pos++;
			command_split_pos = 0;
		}
		else if (!strcmp(argv[index], ";") || ft_arraybilen(argv) < i)
			break ;
		else
		{
			command_pos = -1;
			pipex.exe[pipe_pos].c_split[command_split_pos] = (char *)malloc((ft_strlen(argv[index]) + 1) * sizeof(char));
			while (argv[index][++command_pos])
				pipex.exe[pipe_pos].c_split[command_split_pos][command_pos] = argv[index][command_pos];
			pipex.exe[pipe_pos].c_split[command_split_pos][ft_strlen(argv[index])] = '\0';
			command_split_pos++;
		}
	}
	if (pipex.exe[0].c_split[0])
	{
		if (!strcmp(pipex.exe[0].c_split[0], "cd"))
		{
			if (ft_arraybilen(pipex.exe[0].c_split) > 2 || !pipex.exe[0].c_split[1])
				write (2, "error: cd: bad arguments\n", 25);
			else if (pipex.exe[0].c_split[1] && chdir(pipex.exe[0].c_split[1]))
			{
				write (2, "error: cd: cannot change directory to ", 38);
				write (2, pipex.exe[0].c_split[1], ft_strlen(pipex.exe[0].c_split[1]));
				write (2, "\n", 1);
			}
		}
	}
    if (pipe_pos == 0)
    {
        if ((pid = fork()) == 0)
        {
			if (!pipex.exe[0].c_split[0])
				exit (127);
            else if (execve(pipex.exe[0].c_split[0], pipex.exe[0].c_split, envp) < 0)
            {
                write (2, "error: cannot execute ", 22);
                write (2, pipex.exe[0].c_split[0], ft_strlen(pipex.exe[0].c_split[0]));
                write (2, "\n", 1);
				exit (127);
            }
			exit(0);
        }
        waitpid(0, NULL, 0);
    }
    else
    {
		a = -1;
		fd_in = dup(0);
        while (++a < pipex.procecess_num)
        {
            if (pipe(fd) == -1 || (pid = fork()) == -1)
            {
                write (2, "error: fatal\n", 13);
                exit(0);
            }
            else if (pid == 0)
                child(pipex, envp, a);
            else
            {
				dup2(fd[0], fd_in);
				close(fd[0]);
        		close(fd[1]);
            }
        }
		close(fd_in);
    }
	a = -1;
	while (++a < pipex.procecess_num)
		waitpid(0, NULL, 0);
	a = -1;
	while (++a < pipex.procecess_num)
		if (pipex.exe[a].c_split)
			ft_frlloc(pipex.exe[a].c_split);
	free(pipex.com_count);
	free(pipex.exe);
	return (i);
}

int	count(char **argv, char *c)
{
	int	i;
	int	value;

	i = -1;
	value = 0;
	while (argv[++i])
		if (!strcmp(argv[i], c))
		{
			value++;
			while (ft_arraybilen(argv) > i && !strcmp(argv[i], c))
				i++;
		}
	return (value);
}

int	main(int argc, char **argv, char **envp)
{
	int		dot_comma;
	int		i;

	i = 0;
	if (argc == 1)
		return (0);
	dot_comma = count(argv, ";");
	while (dot_comma >= 0)
	{
		i = ft_execv(argv, i, envp);
		if (ft_arraybilen(argv) <= i || strcmp(argv[i], ";"))
			return (0);
		dot_comma--; 
	}
	return (0);
}
