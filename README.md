# 项目概述

本项目由简单到复杂逐步实现了一个简单的基于epoll的多线程C++服务器。项目采用现代C++11编写，用到无锁队列，线程池，事件驱动设计，使用spdlog库记录日志。介于能力和认知水平有限，目前只实现了基础版本。后续会逐步扩充与优化设计。

## [step1: 搭建基础框架](./docs/step1.md)

## [step2: 添加spdlog日志记录关键信息](./docs/step2.md)

## [step3: 面向对象改造代码](./docs/step3.md)

## [step4：bug定位方法与多线程应用](./docs/step4.md)

## [step5: 设计线程池替代原有多线程](./docs/step5.md)

## [step6: 使用无锁队列改造线程池](./docs/step6.md)

## [step7: boost无锁队列的踩坑与应用](./docs/step7.md)

## [step8: epoll的应用](./docs/step8.md)

## [step9: 继续抽象原有代码](./docs/step9.md)

## [step10: 替代文件描述符改用事件驱动的方式](./docs/step10.md)
