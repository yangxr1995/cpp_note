


# 4大区的内容修改方法
                                                                                      reset --hard <commit id>
                                                                                      reflog
                                                                                     ┌──────┐
                    checkout -- <filename>                 reset HEAD                │      │        push -f
             ┌───────────────────────────────┐    ┌─────────────────────────────┐    │      │ ┌─────────────────────┐
             ▼                               │    ▼                             │    ▼      │ │                     ▼
   ┌──────────────────┐               ┌──────┴──────┐                   ┌───────┴───────────┴─┴┐                 ┌──────────────────┐
   │                  │      add      │             │      commit       │                      │     push        │                  │
   │ workspace(工作区)├──────────────►│Index(暂存区)├──────────────────►│ Repository(本地仓库) ├────────────────►│ Remote(远程仓库) │
   │                  │               │             │                   │                      │◄────────────────│                  │
   └──────────────────┘               └─────────────┘                   └────────┬─────────────┘     clone       └──────────────────┘
              ▲                                                                  │
              └──────────────────────────────────────────────────────────────────┘
                                        checkout

`git checkout -- main.cc` : 工作区中关于main.cc的文件的内容不要了，将暂存区的内容覆盖工作区的main.cc
`git reset <commit id>` : 从暂存区 取内容修改暂存区。不会修改工作区。常用 `git reset HEAD` 取消最近的暂存区修改。
`git reset --hard <commit id>` : 修改 HEAD指针，指向 commit id，撤销本地仓库的错误提交，但实际上不会删除错误提交。撤销后可以使用 `git reflog` 查看所有提交（包括被撤销的提交）
`git push -f` ： 当本地仓库的HEAD落后于远程仓库的HEAD时， `git push` 会报错，必须增加 `-f` 强制push
`git diff HEAD -- <file>` : 查看工作区和 本地仓库中file的差异


# 解决冲突


                      3. add 合并后的代码            4. commit和合并后的代码                     1. push 被拒绝,因为非最新代码                                              
   ┌──────────────────┐               ┌─────────────┐                   ┌──────────────────────┐                                ┌──────────────────┐
   │                  │               │             │                   │                      ├───────────────────────────────►│                  │
   │ workspace(工作区)├──────────────►│Index(暂存区)├──────────────────►│ Repository(本地仓库) ├                                │ Remote(远程仓库) │
   │                  │               │             │                   │                      ├───────────────────────────────►│                  │
   └──────────────────┘               └─────────────┘                   └──────────────────────┘ 5. push 合并后的代码           └─────┬────────────┘
               ▲                                                                                                                      │
               └──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
                  2. pull 拉取最新代码，自动合并本地修改.
                     可能出现两种情况
                     1) 自动合并成功
                     2) 自动合并失败，需要手动合并冲突代码

冲突何时发生 : 当push远程仓库时，可能出现被拒绝的情况，因为有人提交了代码，这时需要 pull最新代码，拉取的代码合并到本地仓库时，若某个文件即被拉取修改，又被自己修改了，就会发生冲突。

当修改不在同一行时, git能自动解决冲突。git会将合并代码输出到工作区，这时用户需要add到暂存区并commit到本地仓库，最后完成push.


```bash
# push冲突
❯ git push -u
To github.com:yangxr1995/git-test.git
 ! [rejected]        main -> main (fetch first)
error: failed to push some refs to 'github.com:yangxr1995/git-test.git'

# 普通的pull没有用
# 因为出现冲突，但git不知道用哪种方式处理冲突
❯ git pull
remote: Enumerating objects: 5, done.
remote: Counting objects: 100% (5/5), done.
remote: Compressing objects: 100% (2/2), done.
remote: Total 3 (delta 0), reused 3 (delta 0), pack-reused 0 (from 0)
Unpacking objects: 100% (3/3), 303 bytes | 101.00 KiB/s, done.
From github.com:yangxr1995/git-test
   91ecb78..068e2f3  main       -> origin/main
hint: You have divergent branches and need to specify how to reconcile them.
hint: You can do so by running one of the following commands sometime before
hint: your next pull:
hint:
hint:   git config pull.rebase false  # merge
hint:   git config pull.rebase true   # rebase
hint:   git config pull.ff only       # fast-forward only
hint:
hint: You can replace "git config" with "git config --global" to set a default
hint: preference for all repositories. You can also pass --rebase, --no-rebase,
hint: or --ff-only on the command line to override the configured default per
hint: invocation.
fatal: Need to specify how to reconcile divergent branches.

# 使用 合并(Merge) 处理
❯ git pull --no-rebase
Auto-merging test
CONFLICT (content): Merge conflict in test
Automatic merge failed; fix conflicts and then commit the result.
# 自动合并失败，手动合并

❯ cat test
init
aaa
<<<<<<< HEAD   # 这是我放的内容
我的代码...
=======        # 这里开始是别人提交的最新代码
被人最新的代码。。。
bbb
>>>>>>> 068e2f39258a4ce74350039a1939b55b129b303a # 这是别人的 commit id

❯ nvim test # 手动编辑文件，进行合并

❯ cat test # 合并后的内容
init
aaa
我的代码...
被人最新的代码。。。
bbb
# 正常提交
❯ git add test
❯ git commit -m "处理冲突"
[main 3bf024f] 处理冲突
❯ git push -u

Enumerating objects: 10, done.
Counting objects: 100% (10/10), done.
Delta compression using up to 8 threads
Compressing objects: 100% (5/5), done.
Writing objects: 100% (6/6), 638 bytes | 638.00 KiB/s, done.
Total 6 (delta 0), reused 0 (delta 0), pack-reused 0
To github.com:yangxr1995/git-test.git
   068e2f3..3bf024f  main -> main
branch 'main' set up to track 'origin/main'.
❯
```


# 本地分支管理
分支用于将新功能的代码和稳定的主线代码分离，当新功能开发完成，合并到主线。

查看分支
```bash
# 查看本地分支
$ git branch
* main

# 查看本地/远程分支
$ git branch -a
* main
  remotes/origin/main

$ git branch -a -vv
* main                3bf024f [origin/main] 处理冲突
  remotes/origin/main 3bf024f 处理冲突

$ git branch -a -v
* main                3bf024f 处理冲突
  remotes/origin/main 3bf024f 处理冲突
```

## 本地dev分支
场景：为了不污染主干代码，先创建dev分支，基于dev分支开发新功能，开发完毕后，将dev分支合并到master分支，最后提交到远程master分支。

```bash
# 创建dev分支，并切换到dev
❯ git checkout -b dev
Switched to a new branch 'dev'
# 实现新功能,并提交
❯ nvim test
❯ git add -u
❯ git commit -m "新功能"
[dev 533dc45] 新功能
 1 file changed, 1 insertion(+)
# 切换到主分支
❯ git checkout main
Switched to branch 'main'
Your branch is up to date with 'origin/main'.
# 确保同步远程分支最新版本
❯ git pull
Already up to date.
# 合并dev分支提交到main分支
❯ git merge dev
Updating 222998a..533dc45
Fast-forward
 test | 1 +
 1 file changed, 1 insertion(+)
# 可见main分支被添加了新的提交，且没有push到远程分支
❯ git status
On branch main
Your branch is ahead of 'origin/main' by 1 commit.
  (use "git push" to publish your local commits)

nothing to commit, working tree clean
# 提交到远程分支
❯ git push origin main
Enumerating objects: 5, done.
Counting objects: 100% (5/5), done.
Delta compression using up to 8 threads
Compressing objects: 100% (3/3), done.
Writing objects: 100% (3/3), 320 bytes | 160.00 KiB/s, done.
Total 3 (delta 1), reused 0 (delta 0), pack-reused 0
remote: Resolving deltas: 100% (1/1), completed with 1 local object.
To github.com:yangxr1995/git-test.git
   222998a..533dc45  main -> main
# 删除dev分支
❯ git branch -d dev
```


     ┌────────────────────────────────────────┐
     │ main   V1 ──────► V2 ─────► V3         │
     │ [orgin/main]                  (HEAD)   │
     └────────────────────────────────────────┘
                                          │
                  git checkout -b dev     │
                                          ▼
     ┌────────────────────────────────────────┐
     │                                (HEAD)  │
     │ dev                ┌──────► V3         │
     │                    │                   │
     │                    │                   │
     │ main   V1 ──────► V2 ─────► V3         │
     │ [origin/main]                          │   
     └────────────────────────────────────────┘
                                          │
                  在dev分支上添加新提交   │
                                          ▼
     ┌────────────────────────────────────────────────┐
     │ dev                ┌──────► V3 ──────► V4      │
     │                    │                     (HEAD)│
     │                    │                           │
     │ main   V1 ──────► V2 ─────► V3                 │
     │ [origin/main]                                  │
     └────────────────────────────────────────────────┘
                                          │
                  git checkout main       │
                                          ▼
     ┌──────────────────────────────────────────────┐
     │ dev                ┌──────► V3 ──────► V4    │
     │                    │                         │
     │                    │                         │
     │ main   V1 ──────► V2 ─────► V3               │
     │ [origin/main]                 (HEAD)         │
     └──────────────────────────────────────────────┘
                                          │
                  git pull                │
                                          ▼
     ┌──────────────────────────────────────────────┐ 
     │ dev                ┌──────► V3 ──────► V4    │ 
     │                    │                         │ 
     │                    │                         │ 
     │ main   V1 ──────► V2 ─────► V3               │ 
     │ [origin/main]                 (HEAD)         │ 
     └──────────────────────────────────────────────┘ 
                                          │
                  git merge dev           │
                                          ▼
     ┌────────────────────────────────────────────────┐
     │ dev                ┌──────► V3 ──────► V4      │
     │                    │                           │
     │                    │                           │
     │ main   V1 ──────► V2 ─────► V3 ──────► V4      │
     │ [origin/main]                           (HEAD) │
     └────────────────────────────────────────────────┘
                                          │
                  git branch -d dev       │
                                          ▼
     ┌────────────────────────────────────────────────┐
     │                                                │
     │ main   V1 ──────► V2 ─────► V3 ──────► V4      │
     │ [origin/main]                           (HEAD) │
     └────────────────────────────────────────────────┘

## 本地分支合并冲突
在合并dev前要在main分支上pull最新代码，可能导致main增加commit，此时合并dev分支就会出现冲突。


     ┌────────────────────────────────────────┐
     │ main   V1 ──────► V2 ─────► V3         │
     │ [orgin/main]                  (HEAD)   │
     └────────────────────────────────────────┘
                                          │
                  git checkout -b dev     │
                                          ▼
     ┌────────────────────────────────────────┐
     │                                (HEAD)  │
     │ dev                ┌──────► V3         │
     │                    │                   │
     │                    │                   │
     │ main   V1 ──────► V2 ─────► V3         │
     │ [origin/main]                          │   
     └────────────────────────────────────────┘
                                          │
                  在dev分支上添加新提交   │
                                          ▼
     ┌────────────────────────────────────────────────┐
     │ dev                ┌──────► V3 ──────► V4      │
     │                    │                     (HEAD)│
     │                    │                           │
     │ main   V1 ──────► V2 ─────► V3                 │
     │ [origin/main]                                  │
     └────────────────────────────────────────────────┘
                                          │
                  git checkout main       │
                                          ▼
     ┌──────────────────────────────────────────────┐
     │ dev                ┌──────► V3 ──────► V4    │
     │                    │                         │
     │                    │                         │
     │ main   V1 ──────► V2 ─────► V3               │
     │ [origin/main]                 (HEAD)         │
     └──────────────────────────────────────────────┘
                                          │
                  git pull                │
                                          ▼
     ┌──────────────────────────────────────────────┐ 
     │ dev                ┌──────► V3 ──────► V4    │ 
     │                    │                         │ 
     │                    │                         │ 
     │ main   V1 ──────► V2 ─────► V3 ──────► V5    │ 
     │ [origin/main]                          (HEAD)│ 
     └──────────────────────────────────────────────┘ 
                                          │
                  git merge dev           │
                                          ▼
     ┌───────────────────────────────────────────────────┐
     │ dev                ┌──────► V3 ──────► V4         │
     │                    │                              │
     │                    │                              │
     │ main   V1 ──────► V2 ─────► V3 ──────► V5         │
     │ [origin/main]                           (HEAD)    │      
     │                                         conflict! │
     └───────────────────────────────────────────────────┘
                                             │
                 处理冲突，提交合并后的代码  │
                                             ▼
     ┌───────────────────────────────────────────────────────────┐
     │ dev                ┌──────► V3 ──────► V4                 │
     │                    │                                      │
     │                    │                                      │
     │ main   v1 ──────► V2 ─────► V3 ──────► V5 ──────► V6      │
     │ [origin/main]                                      (HEAD) │          
     └───────────────────────────────────────────────────────────┘
                                           │
                                           │
                 git branch -d dev         │
                                           ▼
     ┌───────────────────────────────────────────────────────────┐
     │ main   V1 ──────► V2 ─────► V3 ──────► V5 ──────► V6      │
     │ [origin/main]                                      (HEAD) │          
     └───────────────────────────────────────────────────────────┘

# 远程分支管理

远程dev分支的创建需要管理员在git服务器上创建。

git客户端侧，可以创建本地dev分支，然后设置本地dev分支追踪远程 dev分支。

```bash
# 查看远程仓库名
❯ git remote  -vv
origin  git@github.com:yangxr1995/git-test.git (fetch)
origin  git@github.com:yangxr1995/git-test.git (push)
# 创建并切换本地分支，设置追踪的远程分支
git checkout -b <本地分支名> [<远程仓库名>/<远程分支名>]
git checkout -b dev origin/dev
# 设置当前分支追踪的远程分支
git branch -u <远程仓库名>/<远程分支名>
git branch -u origin/dev
```





