/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aamorin- <aamorin-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/06 16:30:05 by aamorin-          #+#    #+#             */
/*   Updated: 2022/01/27 10:06:59 by aamorin-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int	g_return = 0;

typedef struct s_exe
{
	char	**c_split;
}	t_exe;

typedef struct s_pipe
{
	int		**pipes;
	int		*pid;
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
	while (*str)
	{
		str++;
		i++;
	}
	return (i);
}

int	ft_frlloc(char **tab)
{
	int	i;

	i = 0;
	while (tab[i])
	{
		free(tab[i]);
		i++;
	}
	free(tab);
	return (1);
}

void	ft_frlloc_int(int **tab, int size)
{
	int	i;

	i = 0;
	while (size > i)
	{
		free(tab[i]);
		i++;
	}
	free(tab);
}

t_pipe	init_pipex(int a, int i)
{
	t_pipe	pipex;

	pipex.procecess_num = a;
	pipex.pipes = (int **)malloc((pipex.procecess_num + 1) * sizeof(int *));
	pipex.exe = (t_exe *)malloc((pipex.procecess_num + 1) * sizeof(t_exe));
	pipex.pid = (int *)malloc((pipex.procecess_num) * sizeof(int));
	pipex.com_count = (int *)malloc((pipex.procecess_num + 1) * sizeof(int));
	while (++i < pipex.procecess_num + 1)
	{
		pipex.com_count[i] = 0;
		pipex.pipes[i] = (int *)malloc((2) * sizeof(int));
		pipex.exe[i].c_split = NULL;
		if (pipe(pipex.pipes[i]))
		{
			write (2, "error: fatal\n", 13);
			//exit (0);
		}
	}
	return (pipex);
}

void	child(t_pipe pipex, char **envp, int i)
{
	int		j;

	j = 0;
	while (j < pipex.procecess_num + 1)
	{
		if (j != i)
        {
            close(pipex.pipes[j][0]);
        }
		if (j != i + 1)
        {
            close(pipex.pipes[j][1]);
        }
		j++;
	}
	dup2(pipex.pipes[i][0], 0);
	close(pipex.pipes[i][0]);
	dup2(pipex.pipes[i + 1][1], 1);
	close(pipex.pipes[i + 1][1]);
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
	int		pipe_count;
	int		pipe_pos;
	int		command_split_pos;
	int		command_pos;
	int		index;
	int		a;
	int		status;
	t_pipe  pipex;

	pipe_count = 1;
	command_split_pos = 0;
	pipe_pos = 0;
	index = i;
	while (argv[++i])
	{
		if (!strcmp(argv[i], "|"))
			pipe_count++;
		else if (!strcmp(argv[i], ";") || ft_arraybilen(argv) < i)
			break ;
	}
	pipex = init_pipex(pipe_count, -1);
	i = index;
	while (argv[++i])
	{
		if (!strcmp(argv[i], "|"))
			pipe_pos++;
		else if (!strcmp(argv[i], ";" ) || ft_arraybilen(argv) < i)
			break ;
		else
			pipex.com_count[pipe_pos] += 1;
	}
	a = -1;
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
	a = -1;
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
	close(pipex.pipes[0][0]);
	close(pipex.pipes[pipex.procecess_num][1]);
	while (++a < pipex.procecess_num)
	{
		pipex.pid[a] = fork();
		if (pipex.pid[a] == -1)
		{
			write (2, "error: fatal\n", 13);
			exit(0);
		}
		if (pipex.pid[a] == 0)
			child(pipex, envp, a);
	}
	a = -1;
	while (++a < pipex.procecess_num + 1)
	{
        if (a != pipex.procecess_num)
		close(pipex.pipes[a][1]);
		if (a != 0)
			close(pipex.pipes[a][0]);
	}
	a = -1;
	while (++a < pipex.procecess_num)
	{
		waitpid(pipex.pid[a], &status, 0);
		if (WIFEXITED(status))
			g_return = WEXITSTATUS(status);
	}
	a = -1;
	free(pipex.pid);
	while (++a < pipex.procecess_num)
		if (pipex.exe[a].c_split)
			ft_frlloc(pipex.exe[a].c_split);
	ft_frlloc_int(pipex.pipes, pipex.procecess_num + 1);
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
		return (g_return);
	dot_comma = count(argv, ";");
	while (dot_comma >= 0)
	{
		i = ft_execv(argv, i, envp);
        if (ft_arraybilen(argv) <= i || strcmp(argv[i], ";"))
            return (0);
		dot_comma--;
	}
	return (g_return);
}
