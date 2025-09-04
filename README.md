# LLM Centralized System RPG Demo Client

一个基于 UE5 的 **RPG 游戏原型**，展示了 **大语言模型 (LLM) 驱动的智能 NPC、动态对话与关系演化**。  
本仓库是定向研究（Directed Study）的主要部分，重点在于 **客户端侧的对话管线实现、事件系统、提示词工程**。


## 🌟 亮点

- **Agent 对话管线（客户端实现）**  
  - 上下文封装（玩家动作、NPC 状态、场景信息）  
  - 接收 LLM 生成的 **JSON Schema** 输出  
  - 派发事件：NPC 对话、态度变化、关系值更新  

- **事件队列与行为树衔接**  
  - 使用事件队列管理 NPC 交互  
  - 防止“忙碌冲突”，保证动作和对话的连贯性  
  - 与 Unity 行为树逻辑无缝结合  

- **提示词工程 - 角色框架 (Starter / Intermediator / Recipients)**  
  - 明确 NPC 交互角色，保证事件目标与逻辑一致性  
  - 避免对话对象混乱与冲突
 
- **提示词工程 - Persona 保持 & 可控性**  
  - 严格 JSON Schema + Prompt 身份设定  
  - 确保 NPC 在多轮对话中保持角色一致性  
  - 降低幻觉与跑题现象
 


## 📐 架构
![系统架构图](https://github.com/user-attachments/assets/ba0b34da-e0c2-4edc-a81b-e15aa0d117ad)  
*图1：整体架构 – 客户端上下文封装JSON Schema → 发送至LLM → 接收LLM输出 → 事件派发 → NPC 响应*<br><br>

![事件派发系统时序图](https://github.com/user-attachments/assets/691c6dfa-0c38-4f2a-bce7-b77bc5d43035)
*图2：事件队列与 NPC 行为树衔接，保证交互有序*<br><br>

## 📺 演示
![NPC 对话示例1](https://github.com/user-attachments/assets/d14ea426-4d04-4284-9d32-3e0876c6ac5f)
![NPC 对话示例2](https://github.com/user-attachments/assets/d2a01c6b-5808-4c37-bdb6-3a3d8d49b2ca)
*图3 & 4：NPC 对话与关系值演化示例*<br><br>

![NPC 对话示例](/assets/npc-dialogue.png)  
*图5：实机画面*<br><br>


## 🚀 快速开始 (Quick Start)

### 环境
- Unreal Engine 5.5.3

### 使用方法
1. 克隆仓库并在 Unreal 打开 
   ```bash
   git clone https://github.com/yourprofile/llm-rpg-agent-client.git
