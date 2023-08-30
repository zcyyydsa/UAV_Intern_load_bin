High Great Development Framework
===================

## 快速上手

* 克隆代码:
```
git clone git@192.168.1.111:df/hgdf.git
```

* 开始编译</br>有两种方法, 使用`project.py`脚本或者使用原生`CMake`命令
  * 使用`project.py`脚本
    ```
    cd examples/test-hgmav
    # python project.py --toolchain /opt/rk3399pro_hsot/bin --toolchain-prefix aarch64-linux- config
    python project.py menuconfig
    python project.py build
    # python project.py rebuild    # 当删减了文件时, 需要使用此命令而不是 build 命令
    # 可以使用 --verbose 参数来打印编译时的调试信息，在编译出现问题时十分有用
    # python project.py build --verbose
    python project.py clean
    python project.py distclean
    # python project.py clean_conf
    ```
    * 切换工程目录
    * 设置工具链路径以及前缀（如果使用`gcc`不需要设置）
    * 使用命令 `python project.py menuconfig` 来通过终端图形界面配置工程, 这会在 `build/config` 目录生成几个配置文件（`global_config.*`), 我们可以直接在组件(`component`)的`CMakelists.txt` 文件中直接使用（详细看后面的说明）， 或者在 `C/CPP`源文件中通过语句 `#include "global_config.h"` 包含配置头文件来使用
    * 使用命令 `python project.py build` 来启动构建, 可以使用`python project.py build --verbose`命令来打印编译时的调试信息， 在遇到编译错误时十分有用
    * 使用`python project.py clean`来清除构建中间文件, 使用`python project.py distclean`来清楚`menuconfig`命令生成的文件, 这个命令不会清除工具链配置
    * 使用`python project.py clean_conf`来清除工具链配置
  * 使用原生`CMake`命令
    ```
    cd examples/test-hgmav
    # python project.py --toolchain /opt/toolchain/bin --toolchain-prefix aarch64-linux- config
    mkdir build && cd build
    cmake ..
    make menuconfig
    make -j10
    ./build/demo1
    make clean
    rm -rf ./*
    ```
    * 切换工程目录
    * 设置工具链路径以及前缀（如果使用`gcc`不需要设置）
    * 建立一个临时目录并且切换当前路径到这个临时目录（`build`）
    * 使用命令 `cmake ..` 来生成 `Makefile`, 这里 `..` 表示上层目录，即项目的目录
    * 使用命令 `make menuconfig` 来通过终端图形界面配置工程, 这会在 `build/config` 目录生成几个配置文件（`global_config.*`), 我们可以直接在组件(`component`)的`CMakelists.txt` 文件中直接使用（详细看后面的说明）， 或者在 `C/CPP`源文件中通过语句 `#include "global_config.h"` 包含配置头文件来使用
    * 使用命令 `make` 来执行编译链接过程, 或者并行编译： [make -jN](http://www.gnu.org/software/make/manual/make.html#Parallel)， 以及通过 `make VERBOSE=1` 命令来打印编译时的调试信息


## 目录结构

| 目录/文件       | 功能 |
| -------------- | -------- |
| 根目录          | 本项目的根目录，也是 `SDK` 项目的 `SDK` 目录|
| components     | 组件(component)都放在这里 |
| examples       | 工程目录，或者例程目录；在 `SDK` 项目中这个目录是可以和 `SDK` 目录分开放的， 只需要设置环境变量`MY_SDK_PATH`为`SDK`目录路径即可 |
| tools          | 工具目录比如 `cmake`、`kconfig`、`burn tool` 等等 |
| Kconfig        | `Kconfig` 的最顶层配置 |

### 1) 组件（component）

所有库均作为组件(component)被放在`components`目录下或者工程目录下，每个组件用一个目录，这个目录就是这个组件的名字， 为了使工程看起来更简洁，不对组件进行嵌套，所有组件都是一个层级，组件之间的关系依靠依赖关系（requirements）来维持

所有源文件都必须在某个组件内，每个工程必须包含一个叫 `main` 的组件（即`examples/demo1/main` 目录），每个组件包含文件如下：

* `CMakeLists.txt`： 必须有，声明组件源文件以及依赖的组件，并且调用注册函数注册自己，详细可以参考`components/component1`和`components/component2`下`CMakeLists.txt`的写法

* `Kconfig`： 可选，包含了本组件的配置选项， 在本组件或者其它依赖了本组件的组件的`CMakeLists.txt`中都可以在加一个`CONFIG_`前缀后使用这些配置项，比如在`components/component2`中，`Kconfig`中有`COMPONENT2_ENABLED` 选项，我们在它的`CMakeLists.txt`中就使用了这个变量`if(CONFIG_COMPONENT2_ENABLED)`来判断如果用户配置不用这个组件就不注册这个组件

### 2) 工程目录

工程目录在`examples`目录下，当然，这个目录的名字是可以随意根据实际需求修改的，下面可以包含多个实际的工程目录，需要编译那个工程时切换到对应的目录就可以编译。上面也说了，每个工程目录下必须有一个 `main` 组件， 当然也可以放很多自定义的组件。 可以参考`examples/demo1`工程目录。

工程目录下文件：

* `CMakeLists.txt`： 工程属性文件，必须先包含`include(${SDK_PATH}/tools/cmake/compile.cmake)`，然后用`project`函数声明工程名称，比如`project(demo1)`，当然还可以编写其它的条件或者变量等，使用`CMake`语法， 参考`examples/demo1/CMakeLists.txt`的写法

* `config_defaults.mk`： 工程默认配置文件，执行`cmake`构建时会从这里加载默认配置，配置的格式是`Makefile`的格式，可以先使用终端界面配置(`make menuconfig`)生成配置文件复制过来，生成的配置文件在`build/config/global_config.mk`。
> 注意：每次修改`config_defaults.mk` 后需要删除`build`目录下的文件(或者只删除`build/config/global_config.mk`文件)重新生成，因为当前构建系统会优先使用`build`目录下已经存在的配置文件

* `project.py`: 工具脚本调用入口, 使用`python project.py menuconfig` `python project.py build` 等命令来开始构建

如何将工程目录放在磁盘的任何地方: 

* 将`CMakeLists.txt`和 `project.py` 中的 `MY_SDK_PATH` 改成你喜欢的环境变量名称, 然后在终端中设置这个环境变量的值为`SDK`的路径, 即可将这个工程目录放到任何地方也可以编译了

## SDK 和 工程目录分开存放

通常情况下，只需要按照自己的需求，修改`example`目录的名字，比如改成`projects`，或者在工程根目录重新创建一个目录也是可以的，比如`projects/hello_world`，然后拷贝`examples/demo1`中的内容来新建一个新的工程

另外，工程目录和 SDK 目录也可以分开存放，这通常适用于开源项目，一份SDK，用户基于这份 SDK 开发，这样更利于源码传播，用户不需要拷贝一份 SDK， 只需要指定使用的 SDK 版本（git 提交号）。
要做到，只需要：

* 下载 `SDK` 放到某个目录，比如 `/home/hg/my_SDK`

* 然后在终端导出变量 `export MY_SDK_PATH=/home/neucrack/my_SDK`， 可以放到 `~/.bashrc`或者`~/.zshrc`文件中，这样每次终端启动都会自动添加这个变量了
* 然后在任意地方建立工程， 比如拷贝`example/demo1`整个文件夹中的所有文件到`/home/neucrack/temp/my_projects/demo1`
* 然后清除之前的构建缓存（如果有的话，没有就忽略）
```
python3 project.py distclean
```
* 然后配置和构建即可
```
python3 project.py menuconfig
python3 project.py build
```

## 调试版本和发布版本

默认都是以 debug 版本编译，如果要发布版本，可以使用以下命令：
```shell
python project.py distclean
python project.py build --release
```

此时构建的二进制文件就是 release 版本，编译脚本做了几个动作：
* 设置 CMake 环境变量 `CMAKE_BUILD_TYPE` 为 `MinSizeRel`（默认是 `Debug`）
* 在生成的头文件`global_config.h`中添加了 `#define RELEASE 1`(默认会加`#define DEBUG 1`)
* 在编译时自动添加了`RELEASE=1`的宏定义，所以代码其实不用引入`global_config.h`也可以通过`RELEASE`和`DEBUG`宏定义判断当前是 release 版本还是 debug 版本


## 开源许可

**MIT**: 详情看 [LICENSE](./LICENSE) 文件

## 此工程使用到的开源项目

* [Kconfiglib](https://github.com/ulfalizer/Kconfiglib)： `Kconfig` `Python` 实现


