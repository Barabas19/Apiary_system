# 1 Introduction and goals

This document describes the Apiary system, which main goal is to help to beekepers with having adequate information about current condition of the bee families and planning necessary actions based on these informations. As the result this should prevent swarming, decrease infection by Varoa and viruses and increase honey production.

## 1.1 Requirements overview

The Apiary system should be able to safely collect, store and provide the data from the installed sensors (weight scales, temperature sensors, ...). The stored data should be accessible via any web browser in a user-friendly format.
The system should contain the following parts:
- the sensors and data collectors installed in the field
- a database installed on a remote server with API to store/retrieve data
- a web application for access the data from the user side

The Apiary system should be able to:
- collect values from the installed sensors
- store the collected values
- provide a web interface for the user to work with stored data
- operate with aproppriate power supply (bateries in the field)
- provide an interface for calibration of sensors (if necessary)
- detect and log errors, store the logs on a remote server

## 1.2 Quality goals

### 1.2.1 Quality tree

| Quality category | Quality             | Description                                                                     | Scenario |
| ---------------- | ------------------- | ------------------------------------------------------------------------------- | -------- |
| Usablity         | Easy to use         | Easy to use by beekeeper also in the field                                      |          |
|                  | Easy to learn       | The standard functions should be intuitive                                      |          |
| Stability        | Stable in the field | The field parts should provide their functions with no breakdown                | SC1      |
|                  | Web stability       | The web application should provide all the required functions with no breakdown |          |
| Performance      | Accuracy            | The collected values should be correct                                          | SC2      |
|                  | Robustness          | The system should work reliable under all specified conditions                  |          |

### 1.2.2 Quality scenarios

| Id  | Scenario                                                                                                   |
| --- | ---------------------------------------------------------------------------------------------------------- |
| SC1 | Every sensor powered by battery should be able to work at least 4 months without battery change / charging |
| SC2 | The values should be +-1% accurate                                                                         |

## 1.3 Stakeholders

ME :-)

# 2 Constrains

There are the following constraints:
- There can be no Internet connection in the field
- There can be no access to the remote server in the field

# 3 Context and scope

## 3.1 Business context

The following figure shows the inputs and outputs of the Apiary system - the human users and the technical environment.

![Business context](./img/Business_context.png)

## 3.2 Technical context

The technical concepts shows the channels between the units of Apiary system and the users who operate it.

![Technical context](img/Technical_context.png)