# 📚 Applied Review: Making Embedded Systems (Elecia White)

## 📖 Reference

Making Embedded Systems
Elecia White
2ª edição, O’Reilly, 2022.

---

## 📌 Purpose

This document connects theoretical concepts from the book *Making Embedded Systems* by Elecia White with the practical development of the project:

**"Intelligent Instrumentation Dashboard for Autonomous Electric Vehicles with Real-Time Adaptive Monitoring"**

---

# 📌 Chapter 1 — Embedded Systems Fundamentals

## Key Concepts
- Embedded systems are designed for dedicated functions, unlike general-purpose computers  
- Use of cross-compilation for different architectures  
- Constraints in resources (CPU, memory, power) directly impact system design  

## Application in the Project
The system is divided into two main modules:

- **HMI (Human-Machine Interface)**  
  - Raspberry Pi 5 with touch display  

- **Data Acquisition**  
  - ESP32 with sensors  

This structure follows:
- Separation of concerns  
- Low coupling  

## Engineering Decisions
- Use of **C++** for firmware  
- Cross-compilation workflow  
- Early integration testing  
- Monitoring of performance and power consumption  

## Debug Strategy
- Sensor fault detection  
- Communication validation  
- Data consistency checks  

---

# 📌 Chapter 2 — System Architecture

## Key Concepts
- Well-defined architecture is essential for scalable systems  
- Use of modular design and abstraction  

## Architecture Principles
- Encapsulation  
- Low dependency between modules  
- Hardware abstraction  

## Applied Design
- Separation between:
  - Data acquisition  
  - Processing  
  - Interface  

- Identification of critical resources:
  - CAN bus  
  - CPU usage  
  - Data handling  

## Engineering Decisions
- Use of abstraction layers to allow hardware flexibility  
- Modular structure to support future scalability  

---

# 📌 Chapter 3 — Hardware and Integration

## Key Concepts
- Embedded development requires both hardware and software knowledge  
- Hardware constraints influence software design decisions  

## Development Approach
- Component selection based on:
  - Requirements  
  - Cost and availability  
  - Implementation complexity  

- Incremental testing strategy:
  - Individual components → integrated system  

## Application in the Project
- Selection of **Raspberry Pi 5** as the main SBC based on overall balance  

## Best Practices
- Datasheet analysis  
- Early-stage validation  
- Structured integration process  

---

# 🔍 Critical Analysis

The concepts presented in the book directly influenced key architectural decisions in this project, particularly regarding:

- Modular system design
- Hardware abstraction
- Debugging strategies in constrained environments

One key takeaway is the importance of designing systems that remain testable even under hardware limitations — a challenge that is central to this project.




