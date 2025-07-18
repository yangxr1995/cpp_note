cmake_minimum_required(VERSION 3.20)
project(json-test)


set(json_str 
[=[
{
    "webs" : {
        "web" : [
            {
                "name" : "cmake",
                "url" : "cmake.org"
            },
            {
                "name" : "ffmpeg",
                "url" : "ffmpeg.org"
            }
        ]
    }
}
]=])

# JSON
# 1. 读取json项

#    string(JSON <out-var> [ERROR_VARIABLE <error-var>]
#           {GET | TYPE | LENGTH | REMOVE}
#           <json-string> <member|index> [<member|index> ...])
string(JSON name ERROR_VARIABLE evar
    GET ${json_str} webs web 0 name )
message("name : ${name}") # cmake
message("evar : ${evar}") # NOTFOUND

string(JSON url ERROR_VARIABLE evar
    GET ${json_str} webs web 1 url)
message("url : ${url}") # ffmpeg.org
message("evar : ${evar}") # NOTFOUND

string(JSON url ERROR_VARIABLE evar
    GET ${json_str} webs web 2 url)
message("url : ${url}") # webs-web-2-NOTFOUND
message("evar : ${evar}") # expected an index less than 2 got '2'


# 2. 获得json数组长度
string(JSON arr_len ERROR_VARIABLE evar
    LENGTH ${json_str} webs web)
message("arr_len : ${arr_len}") # 2
message("evar : ${evar}") # NOTFOUND

# 3. 添加/修改
#  string(JSON <out-var> [ERROR_VARIABLE <error-var>]
#          SET <json-string>
#          <member|index> [<member|index> ...] <value>)
set(new_item 
[=[
{
    "name" : "aa",
    "url" : "aa.org"
}
]=])
string(JSON json_str_new ERROR_VARIABLE evar
    SET ${json_str} webs web ${arr_len} ${new_item})

# {
#   "webs" : 
#   {
#     "web" : 
#     [
#       {
#         "name" : "cmake",
#         "url" : "cmake.org"
#       },
#       {
#         "name" : "ffmpeg",
#         "url" : "ffmpeg.org"
#       },
#       {
#         "name" : "aa",
#         "url" : "aa.org"
#       }
#     ]
#   }
# }
message("json_str_new : ${json_str_new}")

set(new_item
[=[
{
    "name" : "bb",
    "url" : "bb.org",
}
]=])
string(JSON json_str_new ERROR_VARIABLE evar
    SET ${json_str_new} webs web ${arr_len} ${new_item})
# {
#   "webs" : 
#   {
#     "web" : 
#     [
#       {
#         "name" : "cmake",
#         "url" : "cmake.org"
#       },
#       {
#         "name" : "ffmpeg",
#         "url" : "ffmpeg.org"
#       },
#       {
#         "name" : "bb",
#         "url" : "bb.org"
#       }
#     ]
#   }
# }
message("json_str_new : ${json_str_new}")

# 4. 删除
#  string(JSON <out-var> [ERROR_VARIABLE <error-variable>]
#         REMOVE <json-string> <member|index> [<member|index> ...])
#
string(JSON json_str_new ERROR_VARIABLE evar
    REMOVE ${json_str_new} webs web 0)
# {
#   "webs" : 
#   {
#     "web" : 
#     [
#       {
#         "name" : "ffmpeg",
#         "url" : "ffmpeg.org"
#       },
#       {
#         "name" : "bb",
#         "url" : "bb.org"
#       }
#     ]
#   }
# }
message("json_str_new : ${json_str_new}")
