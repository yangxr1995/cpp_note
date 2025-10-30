


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




# 写注释

推荐采用「一行标题 + 可选详细描述」的格式，标题与描述之间空一行：
```bash
plaintext
<类型>[可选作用域]: <简短描述>
[可选详细描述]
[可选脚注]
```

各部分说明
类型（Type）说明提交的性质，常用类型：
- feat：新功能（feature）
- fix：修复 bug
- docs：仅文档变更
- style：代码格式调整（不影响代码逻辑）
- refactor：代码重构（既不是新功能也不是修 bug）
- perf：性能优化
- test：添加或修改测试代码
- chore：构建过程或辅助工具的变动

作用域（Scope）可选，说明提交影响的模块或范围，例如：
```bash
plaintext
feat(login): 添加验证码登录功能
fix(payment): 修复订单支付超时问题
```

在 Linux 终端中使用 Git 提交（commit）时添加注释，主要通过 git commit 命令的参数来实现，常用方法如下：
1. 直接在命令行添加简短注释
使用 -m 参数可以直接在命令后跟上提交注释，适合简单的提交：
```bash
git commit -m "feat(login): 添加记住密码功能"
```
注释内容需用引号（单引号或双引号）包裹
建议遵循前面提到的 commit 注释规范
2. 打开编辑器编写详细注释
直接运行 git commit（不带 -m 参数），会自动打开系统默认编辑器（通常是 vim 或 nano）来编写注释：
```bash
git commit
```
vim 编辑器操作：
进入后按 i 键进入编辑模式
编写注释（第一行为标题，空一行后写详细描述）
按 Esc 退出编辑模式，输入 :wq 保存并退出
如果想修改默认编辑器：可以通过以下命令设置（以 VS Code 为例）：
```bash
git config --global core.editor "code --wait"
```
3. 补充修改后提交（无需重复写注释）
如果提交后发现漏改了内容，修改后可以用 --amend 补充到上一次提交，且可修改注释：
bash
# 修改后补充提交，会打开编辑器让你修改原有注释
```bash
git commit --amend
```

# 或者直接指定新注释覆盖原有注释
```bash
git commit --amend -m "fix(login): 修复记住密码功能失效问题"
```
这些方法可以满足不同场景的注释需求，简单变更用 -m 快速提交，复杂变更建议用编辑器编写详细注释，便于后续查阅。
如果commit 内容有换行怎么处理
在 Git 提交（commit）时需要添加换行的注释内容，可以通过以下几种方式处理：
1. 使用 -m 参数多次添加换行
在终端中用 -m 参数多次指定内容，Git 会自动将它们用换行分隔：
```bash
git commit -m "feat: 优化用户注册流程" \
-m "1. 增加邮箱格式验证" \
-m "2. 优化密码强度提示" \
-m "3. 修复验证码过期不刷新问题"
```
每行用 -m 开头，使用反斜杠 \ 连接多行（可选，仅为终端命令换行美观）
最终提交信息会包含这些换行内容
2. 用引号包裹包含换行符的内容
在单引号或双引号中直接使用 Enter 键换行（适合简单的多行注释）：
```bash
git commit -m 'feat: 优化用户注册流程
1. 增加邮箱格式验证
2. 优化密码强度提示
3. 修复验证码过期不刷新问题'
```
输入时在引号内按 Enter 即可换行
注意引号要成对，且结尾的引号要在最后一行结束后添加
3. 使用编辑器编写多行注释（推荐）
直接运行 git commit 命令，会打开默认编辑器（如 vim、nano），可以自由换行编写：
```bash
git commit
```
在编辑器中：
第一行写标题（简短描述）
空一行后开始写详细描述，可自由换行
编写完成后保存退出即可
vim 编辑器示例：
```plaintext
feat: 优化用户注册流程

1. 增加邮箱格式验证
2. 优化密码强度提示
3. 修复验证码过期不刷新问题

Closes #789
```
这种方式最适合编写复杂的多行注释，且能更好地遵循规范格式。
总结
简单多行用 -m 多次指定或引号内换行
复杂多行推荐用编辑器编写（git commit 无参数方式）
无论哪种方式，都建议遵循「标题 + 空行 + 详细描述」的结构，保持提交历史清晰。

## git diff
- git diff（不加参数）：显示工作区与暂存区的差异
- git diff --cached：显示暂存区与HEAD的差异
- git diff HEAD：显示工作区与HEAD的差异（包括未暂存的变更）
