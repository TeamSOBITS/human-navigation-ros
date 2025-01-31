#include <rclcpp/rclcpp.hpp>
#include <iostream>
#include "human_navigation/msg/human_navi_object_info.hpp"
#include "human_navigation/msg/human_navi_destination.hpp"
#include "human_navigation/msg/human_navi_task_info.hpp"
#include "human_navigation/msg/human_navi_msg.hpp"
#include "human_navigation/msg/human_navi_guidance_msg.hpp"
#include "human_navigation/msg/human_navi_avatar_status.hpp"
#include "human_navigation/msg/human_navi_object_status.hpp"

class HumanNavigationSample : public rclcpp::Node
{
private:
	enum Step
	{
		Initialize,
		Ready,
		WaitTaskInfo,
		GuideForTakingObject,
		GuideForPlacement,
		WaitTaskFinished,
		TaskFinished
	};

	enum class SpeechState
	{
		None,
		WaitingState,
		Speaking,
		Speakable
	};

	// human navigation message from/to the moderator
	const std::string MSG_ARE_YOU_READY      = "Are_you_ready?";
	const std::string MSG_TASK_SUCCEEDED     = "Task_succeeded";
	const std::string MSG_TASK_FAILED        = "Task_failed";
	const std::string MSG_TASK_FINISHED      = "Task_finished";
	const std::string MSG_GO_TO_NEXT_SESSION = "Go_to_next_session";
	const std::string MSG_MISSION_COMPLETE   = "Mission_complete";
	const std::string MSG_REQUEST            = "Guidance_request";
	const std::string MSG_SPEECH_STATE       = "Speech_state";
	const std::string MSG_SPEECH_RESULT      = "Speech_result";

	const std::string MSG_I_AM_READY        = "I_am_ready";
	const std::string MSG_GET_AVATAR_STATUS = "Get_avatar_status";
	const std::string MSG_GET_OBJECT_STATUS = "Get_object_status";
	const std::string MSG_GET_SPEECH_STATE  = "Get_speech_state";

	// display type of guidance message panels for the avatar (test subject)
	const std::string DISPLAY_TYPE_ALL         = "All";
	const std::string DISPLAY_TYPE_ROBOT_ONLY  = "RobotOnly";
	const std::string DISPLAY_TYPE_AVATAR_ONLY = "AvatarOnly";
	const std::string DISPLAY_TYPE_NONE        = "None";

	int step;
	SpeechState speechState;

	bool isStarted;
	bool isFinished;

	bool isTaskInfoReceived;
	bool isRequestReceived;

	rclcpp::Time timePrevSpeechStateConfirmed;

	bool isSentGetAvatarStatus;
	bool isSentGetObjectStatus;

	human_navigation::msg::HumanNaviTaskInfo taskInfo;
	std::string guideMsg;

	human_navigation::msg::HumanNaviAvatarStatus avatarStatus;
	human_navigation::msg::HumanNaviObjectStatus objectStatus;

	void init() {
		step = Initialize;
		speechState = SpeechState::None;

		reset();
	}

	void reset() {
		isStarted             = false;
		isFinished            = false;
		isTaskInfoReceived    = false;
		isRequestReceived     = false;
		isSentGetAvatarStatus = false;
		isSentGetObjectStatus = false;
	}

	// send humanNaviMsg to the moderator (Unity)
	void sendMessage(rclcpp::Publisher<human_navigation::msg::HumanNaviMsg>::SharedPtr publisher, const std::string &message) {
		human_navigation::msg::HumanNaviMsg human_navi_msg;
		human_navi_msg.message = message;
		publisher->publish(human_navi_msg);

		RCLCPP_INFO(this->get_logger(), "Send message:%s", message.c_str());
	}

	void sendGuidanceMessage(rclcpp::Publisher<human_navigation::msg::HumanNaviGuidanceMsg>::SharedPtr publisher, const std::string &message, const std::string displayType) {
		human_navigation::msg::HumanNaviGuidanceMsg guidanceMessage;
		guidanceMessage.message = message;
		guidanceMessage.display_type = displayType;
		guidanceMessage.source_language = ""; // Blank or ISO-639-1 language code, e.g. "en".
		guidanceMessage.target_language = ""; // Blank or ISO-639-1 language code, e.g. "ja".
		publisher->publish(guidanceMessage);

		speechState = SpeechState::Speaking;

		RCLCPP_INFO(this->get_logger(), "Send guide message: %s : %s", guidanceMessage.message.c_str(), guidanceMessage.display_type.c_str());
	}


	// receive humanNaviMsg from the moderator (Unity)
	void messageCallback(const human_navigation::msg::HumanNaviMsg::ConstPtr& message) {
		RCLCPP_INFO(this->get_logger(), "Subscribe message: %s : %s", message->message.c_str(), message->detail.c_str());

		if(message->message==MSG_ARE_YOU_READY)	isStarted = true;
		else if(message->message==MSG_REQUEST) {
			if(isTaskInfoReceived && !isFinished) isRequestReceived = true;
		}
		else if(message->message==MSG_TASK_SUCCEEDED) {
		}
		else if(message->message==MSG_TASK_FAILED) {
		}
		else if(message->message==MSG_TASK_FINISHED) isFinished = true;
		else if(message->message==MSG_GO_TO_NEXT_SESSION) {
			RCLCPP_INFO(this->get_logger(), "Go to next session");
			step = Initialize;
		}
		else if(message->message==MSG_MISSION_COMPLETE) {
			//exit(EXIT_SUCCESS);
		}
		else if(message->message==MSG_SPEECH_STATE) {
			if(message->detail=="Is_speaking") speechState = SpeechState::Speaking;
			else speechState = SpeechState::Speakable;
		}
		else if(message->message==MSG_SPEECH_RESULT) RCLCPP_INFO(this->get_logger(), "Speech result: %s", message->detail.c_str());
	}

	// receive taskInfo from the moderator (Unity)
	void taskInfoMessageCallback(const human_navigation::msg::HumanNaviTaskInfo::ConstPtr& message) {
		taskInfo = *message;
		int numOfNonTargetObjects = taskInfo.non_target_objects.size();

		RCLCPP_INFO_STREAM(this->get_logger(),
			"Subscribe task info message:" << std::endl <<
			"Environment ID: " << taskInfo.environment_id << std::endl <<
			"Target object: " << std::endl <<
			"  Name: " << taskInfo.target_object.name << std::endl <<
			"  Position: (" 
			<< taskInfo.target_object.position.x << ", "
			<< taskInfo.target_object.position.y << ", "
			<< taskInfo.target_object.position.z << ")" << std::endl <<
			"  Orientation: (" 
			<< taskInfo.target_object.orientation.x << ", "
			<< taskInfo.target_object.orientation.y << ", "
			<< taskInfo.target_object.orientation.z << ", "
			<< taskInfo.target_object.orientation.w << ")" << std::endl <<
			"Destination: " << std::endl <<
			"  Position: (" 
			<< taskInfo.destination.position.x << ", "
			<< taskInfo.destination.position.y << ", "
			<< taskInfo.destination.position.z << ")" << std::endl <<
			"  Orientation: (" 
			<< taskInfo.destination.orientation.x << ", "
			<< taskInfo.destination.orientation.y << ", "
			<< taskInfo.destination.orientation.z << ", "
			<< taskInfo.destination.orientation.w << ")"
		);

		RCLCPP_INFO_STREAM(this->get_logger(), 
			"Number of non-target objects: " << numOfNonTargetObjects << std::endl << 
			"Non-target objects:" << std::endl
		);

		for(int i=0; i<numOfNonTargetObjects; i++) {
			RCLCPP_INFO_STREAM(this->get_logger(),
				"Non-target object " << i << ": " << std::endl <<
				"  Name: " << taskInfo.non_target_objects[i].name << std::endl <<
				"  Position: (" 
				<< taskInfo.non_target_objects[i].position.x << ", "
				<< taskInfo.non_target_objects[i].position.y << ", "
				<< taskInfo.non_target_objects[i].position.z << ")" << std::endl <<
				"  Orientation: (" 
				<< taskInfo.non_target_objects[i].orientation.x << ", "
				<< taskInfo.non_target_objects[i].orientation.y << ", "
				<< taskInfo.non_target_objects[i].orientation.z << ", "
				<< taskInfo.non_target_objects[i].orientation.w << ")"
			);
		}

		int numOfFurniture = taskInfo.furniture.size();
		RCLCPP_INFO_STREAM(this->get_logger(), 
			"Number of furniture: " << numOfFurniture << std::endl << 
			"Furniture objects:" << std::endl
		);
		for(int i=0; i<numOfFurniture; i++) {
			RCLCPP_INFO_STREAM(this->get_logger(),
				"Furniture " << i << ": " << std::endl <<
				"  Name: " << taskInfo.furniture[i].name << std::endl <<
				"  Position: (" 
				<< taskInfo.furniture[i].position.x << ", "
				<< taskInfo.furniture[i].position.y << ", "
				<< taskInfo.furniture[i].position.z << ")" << std::endl <<
				"  Orientation: (" 
				<< taskInfo.furniture[i].orientation.x << ", "
				<< taskInfo.furniture[i].orientation.y << ", "
				<< taskInfo.furniture[i].orientation.z << ", "
				<< taskInfo.furniture[i].orientation.w << ")"
			);
			
		}

		isTaskInfoReceived = true;
	}

	void avatarStatusMessageCallback(const human_navigation::msg::HumanNaviAvatarStatus::ConstPtr& message) {
		avatarStatus = *message;

		RCLCPP_INFO_STREAM(this->get_logger(),
			"Subscribe avatar status message:" << std::endl <<
			"Head: " << std::endl <<
			"  Position: (" 
			<< avatarStatus.head.position.x << ", "
			<< avatarStatus.head.position.y << ", "
			<< avatarStatus.head.position.z << ")" << std::endl <<
			"  Orientation: (" 
			<< avatarStatus.head.orientation.x << ", "
			<< avatarStatus.head.orientation.y << ", "
			<< avatarStatus.head.orientation.z << ", "
			<< avatarStatus.head.orientation.w << ")" << std::endl <<
			"Left Hand: " << std::endl <<
			"  Position: (" 
			<< avatarStatus.left_hand.position.x << ", "
			<< avatarStatus.left_hand.position.y << ", "
			<< avatarStatus.left_hand.position.z << ")" << std::endl <<
			"  Orientation: (" 
			<< avatarStatus.left_hand.orientation.x << ", "
			<< avatarStatus.left_hand.orientation.y << ", "
			<< avatarStatus.left_hand.orientation.z << ", "
			<< avatarStatus.left_hand.orientation.w << ")" << std::endl <<
			"Right Hand: " << std::endl <<
			"  Position: (" 
			<< avatarStatus.right_hand.position.x << ", "
			<< avatarStatus.right_hand.position.y << ", "
			<< avatarStatus.right_hand.position.z << ")" << std::endl <<
			"  Orientation: (" 
			<< avatarStatus.right_hand.orientation.x << ", "
			<< avatarStatus.right_hand.orientation.y << ", "
			<< avatarStatus.right_hand.orientation.z << ", "
			<< avatarStatus.right_hand.orientation.w << ")" << std::endl <<
			"Object in Left Hand: " << avatarStatus.object_in_left_hand << std::endl <<
			"Object in Right Hand: " << avatarStatus.object_in_right_hand << std::endl <<
			"Is Target Object in Left Hand: " << std::boolalpha << avatarStatus.is_target_object_in_left_hand << std::endl <<
			"Is Target Object in Right Hand: " << std::boolalpha << avatarStatus.is_target_object_in_right_hand
		);

		isSentGetAvatarStatus = false;
	}

	void objectStatusMessageCallback(const human_navigation::msg::HumanNaviObjectStatus::ConstPtr& message) {
		objectStatus = *message;

		RCLCPP_INFO_STREAM(this->get_logger(),
			"Subscribe object status message:" << std::endl <<
			"Target object: " << std::endl <<
			"  Name: " << taskInfo.target_object.name << std::endl <<
			"  Position: (" 
			<< taskInfo.target_object.position.x << ", "
			<< taskInfo.target_object.position.y << ", "
			<< taskInfo.target_object.position.z << ")" << std::endl <<
			"  Orientation: (" 
			<< taskInfo.target_object.orientation.x << ", "
			<< taskInfo.target_object.orientation.y << ", "
			<< taskInfo.target_object.orientation.z << ", "
			<< taskInfo.target_object.orientation.w << ")"
		);

		int numOfNonTargetObjects = taskInfo.non_target_objects.size();
		RCLCPP_INFO_STREAM(this->get_logger(), 
			"Number of non-target objects: " << numOfNonTargetObjects << std::endl << 
			"Non-target objects:" << std::endl
		);
		for(int i=0; i<numOfNonTargetObjects; i++) {
			RCLCPP_INFO_STREAM(this->get_logger(),
				"Non-target object " << i << ": " << std::endl <<
				"  Name: " << taskInfo.non_target_objects[i].name << std::endl <<
				"  Position: (" 
				<< taskInfo.non_target_objects[i].position.x << ", "
				<< taskInfo.non_target_objects[i].position.y << ", "
				<< taskInfo.non_target_objects[i].position.z << ")" << std::endl <<
				"  Orientation: (" 
				<< taskInfo.non_target_objects[i].orientation.x << ", "
				<< taskInfo.non_target_objects[i].orientation.y << ", "
				<< taskInfo.non_target_objects[i].orientation.z << ", "
				<< taskInfo.non_target_objects[i].orientation.w << ")"
			);
		}

		isSentGetObjectStatus = false;
	}

	bool speakGuidanceMessage(rclcpp::Publisher<human_navigation::msg::HumanNaviMsg>::SharedPtr pubHumanNaviMsg, 
                          rclcpp::Publisher<human_navigation::msg::HumanNaviGuidanceMsg>::SharedPtr pubGuidanceMsg, 
                          std::string message, int interval = 1) {
    if(speechState == SpeechState::Speakable) {
        sendGuidanceMessage(pubGuidanceMsg, message, DISPLAY_TYPE_ALL);
        speechState = SpeechState::None;
        return true;
    }
    else if(speechState == SpeechState::None || speechState == SpeechState::Speaking) {
        if(timePrevSpeechStateConfirmed.seconds() + interval < rclcpp::Clock(RCL_SYSTEM_TIME).now().seconds()) {
            sendMessage(pubHumanNaviMsg, MSG_GET_SPEECH_STATE);
            timePrevSpeechStateConfirmed = rclcpp::Clock(RCL_SYSTEM_TIME).now();
            speechState = SpeechState::WaitingState;
        }
    }

    return false;
}

public:
  HumanNavigationSample() : Node("human_navigation_sample") {
  }
	int run(int argc, char **argv) {

		rclcpp::Rate loopRate(10);

		init();

		RCLCPP_INFO(this->get_logger(), "Human Navi sample start!");

    std::string sub_msg_to_robot_topic_name = "/human_navigation/message/to_robot";
    std::string sum_msg_task_info_topic_name = "/human_navigation/message/task_info";
    std::string sub_msg_avatar_status_topic_name = "/human_navigation/message/avatar_status";
    std::string sum_msg_object_status_topic_name = "/human_navigation/message/object_status";
    std::string pub_msg_to_moderator_topic_name = "/human_navigation/message/to_moderator";
    std::string pub_msg_guidance_message_topic_name = "/human_navigation/message/guidance_message";

    this->declare_parameter<std::string>("sub_msg_to_robot_topic_name", "/human_navigation/message/to_robot");
    this->declare_parameter<std::string>("sum_msg_task_info_topic_name", "/human_navigation/message/task_info");
    this->declare_parameter<std::string>("sub_msg_avatar_status_topic_name", "/human_navigation/message/avatar_status");
    this->declare_parameter<std::string>("sum_msg_object_status_topic_name", "/human_navigation/message/object_status");
    this->declare_parameter<std::string>("pub_msg_to_moderator_topic_name", "/human_navigation/message/to_moderator");
    this->declare_parameter<std::string>("pub_msg_guidance_message_topic_name", "/human_navigation/message/guidance_message");

    this->get_parameter("sub_msg_to_robot_topic_name", sub_msg_to_robot_topic_name);
    this->get_parameter("sum_msg_task_info_topic_name", sum_msg_task_info_topic_name);
    this->get_parameter("sub_msg_avatar_status_topic_name", sub_msg_avatar_status_topic_name);
    this->get_parameter("sum_msg_object_status_topic_name", sum_msg_object_status_topic_name);
    this->get_parameter("pub_msg_to_moderator_topic_name", pub_msg_to_moderator_topic_name);
    this->get_parameter("pub_msg_guidance_message_topic_name", pub_msg_guidance_message_topic_name);


    auto subHumanNaviMsg = this->create_subscription<human_navigation::msg::HumanNaviMsg>(sub_msg_to_robot_topic_name, 100, std::bind(&HumanNavigationSample::messageCallback, this, std::placeholders::_1));
    auto subTaskInfoMsg = this->create_subscription<human_navigation::msg::HumanNaviTaskInfo>(sum_msg_task_info_topic_name, 100, std::bind(&HumanNavigationSample::taskInfoMessageCallback, this, std::placeholders::_1));
    auto subAvatarStatusMsg = this->create_subscription<human_navigation::msg::HumanNaviAvatarStatus>(sub_msg_avatar_status_topic_name, 100, std::bind(&HumanNavigationSample::avatarStatusMessageCallback, this, std::placeholders::_1));
    auto subObjectStatusMsg = this->create_subscription<human_navigation::msg::HumanNaviObjectStatus>(sum_msg_object_status_topic_name, 100, std::bind(&HumanNavigationSample::objectStatusMessageCallback, this, std::placeholders::_1));
    auto pubHumanNaviMsg = this->create_publisher<human_navigation::msg::HumanNaviMsg>(pub_msg_to_moderator_topic_name, 10);
    auto pubGuidanceMsg = this->create_publisher<human_navigation::msg::HumanNaviGuidanceMsg>(pub_msg_guidance_message_topic_name, 10);

		rclcpp::Time time;

		while (rclcpp::ok()) {
			switch(step) {
				case Initialize: {
					reset();

					RCLCPP_INFO(this->get_logger(), "##### Initialized ######");

					step++;
					break;
				}
				case Ready: {
					if(isStarted) {
						step++;

						sendMessage(pubHumanNaviMsg, MSG_I_AM_READY);

						RCLCPP_INFO(this->get_logger(), "Task start");
					}
					break;
				}
				case WaitTaskInfo: {
					if(isTaskInfoReceived){ step++; }
					break;
				}
				case GuideForTakingObject: {
					if(isRequestReceived) isRequestReceived = false;

					std::string targetObjectName;
					if(taskInfo.target_object.name.find("empty_plastic_bottle") != std::string::npos) targetObjectName = "an empty plastic bottle ";
					else targetObjectName = "a cup ";

					std::string locationName;
					if(taskInfo.target_object.position.x > 0.0) locationName = "on a table.";
					else locationName = "next to the kitchen sink.";

					guideMsg = "Please take " + targetObjectName + locationName;

					if(speakGuidanceMessage(pubHumanNaviMsg, pubGuidanceMsg, guideMsg)) {
						time = rclcpp::Clock(RCL_SYSTEM_TIME).now();;
						step++;
					}
					break;
				}
				case GuideForPlacement: {
					if(isRequestReceived) {
						if(speakGuidanceMessage(pubHumanNaviMsg, pubGuidanceMsg, guideMsg)) isRequestReceived = false;
					}

					int WaitTime = 5;
          rclcpp::Time current_time = rclcpp::Clock(RCL_SYSTEM_TIME).now();

					if(time.seconds() + WaitTime < current_time.seconds()) {
						std::string destinationName;
						if(taskInfo.destination.position.z < 1.0) destinationName = "a trash can on the left.";
						else destinationName = "the second cabinet from the right.";
						guideMsg = "Put it in " + destinationName;

						if(speakGuidanceMessage(pubHumanNaviMsg, pubGuidanceMsg, guideMsg)) {
							time = rclcpp::Clock(RCL_SYSTEM_TIME).now();;
							step++;
						}
					}

					break;
				}
				case WaitTaskFinished: {
					if(isFinished) {
						RCLCPP_INFO(this->get_logger(), "Task finished");
						step++;
						break;
					}

					if(isRequestReceived) {
						bool isSpeaked;
						if(static_cast<int>(rclcpp::Clock(RCL_SYSTEM_TIME).now().seconds()) % 2 > 0) isSpeaked = speakGuidanceMessage(pubHumanNaviMsg, pubGuidanceMsg, guideMsg);
						else isSpeaked = speakGuidanceMessage(pubHumanNaviMsg, pubGuidanceMsg, "You can find the wall cabinet above the kitchen sink.");

						if(isSpeaked) isRequestReceived = false;
					}

					int WaitTime = 5;
					if(time.seconds() + WaitTime < static_cast<int>(rclcpp::Clock(RCL_SYSTEM_TIME).now().seconds())) {
						if(!isSentGetAvatarStatus && !isSentGetObjectStatus) {
							sendMessage(pubHumanNaviMsg, MSG_GET_AVATAR_STATUS);
							sendMessage(pubHumanNaviMsg, MSG_GET_OBJECT_STATUS);
							isSentGetAvatarStatus = true;
							isSentGetObjectStatus = true;
							time = rclcpp::Clock(RCL_SYSTEM_TIME).now();
						}
					}

					break;
				}
				case TaskFinished: {
					// Wait MSG_GO_TO_NEXT_SESSION or MSG_MISSION_COMPLETE
					break;
				}
			}

      rclcpp::spin_some(this->get_node_base_interface());

			loopRate.sleep();
		}

		return 0;
	}
};

int main(int argc, char **argv) {
	rclcpp::init(argc, argv);

	HumanNavigationSample humanNaviSample;

	humanNaviSample.run(argc, argv);
};