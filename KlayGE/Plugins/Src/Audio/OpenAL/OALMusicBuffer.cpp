// OALMusicBuffer.cpp
// KlayGE OpenAL音乐缓冲区类 实现文件
// Ver 2.2.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.2.0
// 修正了DoStop的死锁 (2004.10.22)
//
// 2.0.4
// 增加了循环播放功能 (2004.3.22)
//
// 2.0.0
// 初次建立 (2003.7.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <cassert>

#include <KlayGE/OpenAL/OALAudio.hpp>

size_t const READSIZE(88200);

namespace
{
	// 处理时间事件
	/////////////////////////////////////////////////////////////////////////////////
	class PlayProc
	{
	public:
		PlayProc(KlayGE::OALMusicBuffer* streaming)
					: streaming_(streaming)
			{ }

		void operator()()
		{
			// 更新缓冲区
			streaming_->LoopUpdateBuffer();
		}

	private:
		KlayGE::OALMusicBuffer* streaming_;
	};
}

namespace KlayGE
{
	// 构造函数。建立一个可以用于流式播放的缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	OALMusicBuffer::OALMusicBuffer(AudioDataSourcePtr const & dataSource, uint32 bufferSeconds, float volume)
							: MusicBuffer(dataSource),
								bufferQueue_(bufferSeconds * PreSecond)
	{
		alGenBuffers(static_cast<ALsizei>(bufferQueue_.size()), &bufferQueue_[0]);

		alGenSources(1, &source_);
		alSourcef(source_, AL_PITCH, 1);

		this->Position(Vector3(0, 0, 0.1f));
		this->Velocity(Vector3(0, 0, 0));
		this->Direction(Vector3(0, 0, 0));

		this->Volume(volume);

		this->Reset();
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	OALMusicBuffer::~OALMusicBuffer()
	{
		this->Stop();

		alDeleteBuffers(static_cast<ALsizei>(bufferQueue_.size()), &bufferQueue_[0]);
		alDeleteSources(1, &source_);
	}

	// 更新缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::LoopUpdateBuffer()
	{
		ALint processed;
		bool finishedPlaying(false);
		size_t buffersInQueue(bufferQueue_.size());
		ALuint buf;

		while (!finishedPlaying)
		{
			alGetSourcei(source_, AL_BUFFERS_PROCESSED, &processed);
			if (processed > 0)
			{
				while (processed > 0)
				{
					-- processed;

					alSourceUnqueueBuffers(source_, 1, &buf);

					std::vector<uint8> data(READSIZE);
					data.resize(dataSource_->Read(&data[0], data.size()));
					if (!data.empty())
					{
						alBufferData(buf, Convert(format_), &data[0],
							static_cast<ALsizei>(data.size()), freq_);
						alSourceQueueBuffers(source_, 1, &buf);
					}
					else
					{
						-- buffersInQueue;

						if (0 == buffersInQueue)
						{
							if (loop_)
							{
								this->Reset();
								this->Play(true);
							}
							else
							{
								finishedPlaying = true;
								break;
							}
						}
					}
				}
			}
			else
			{
				Sleep(500 / this->PreSecond);
			}
		}
	}

	// 缓冲区复位以便于从头播放
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::DoReset()
	{
		alSourceUnqueueBuffers(source_, static_cast<ALsizei>(bufferQueue_.size()), &bufferQueue_[0]);

		ALenum const format(Convert(format_));
		std::vector<uint8> data(READSIZE);

		// 每个缓冲区中装1 / PreSecond秒的数据
		for (BuffersIter iter = bufferQueue_.begin(); iter != bufferQueue_.end(); ++ iter)
		{
			dataSource_->Read(&data[0], data.size());
			alBufferData(*iter, format, &data[0], static_cast<ALuint>(data.size()), freq_);
		}

		alSourceQueueBuffers(source_, static_cast<ALsizei>(bufferQueue_.size()), &bufferQueue_[0]);

		alSourceRewindv(1, &source_);
	}

	// 播放音频流
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::DoPlay(bool loop)
	{
		playThread_.reset(new boost::thread(PlayProc(this)));

		loop_ = loop;

		alSourcei(source_, AL_LOOPING, false);
		alSourcePlay(source_);
	}

	// 停止播放音频流
	////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::DoStop()
	{
		if (playThread_)
		{
			playThread_.reset();
		}

		alSourceStopv(1, &source_);
	}

	// 检查缓冲区是否在播放
	/////////////////////////////////////////////////////////////////////////////////
	bool OALMusicBuffer::IsPlaying() const
	{
		ALint value;
		alGetSourcei(source_, AL_SOURCE_STATE, &value);

		return (AL_PLAYING == (value & AL_PLAYING));
	}

	// 设置音量
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::Volume(float vol)
	{
		alSourcef(source_, AL_GAIN, vol);
	}

	// 获取声源位置
	/////////////////////////////////////////////////////////////////////////////////
	Vector3 OALMusicBuffer::Position() const
	{
		float pos[3];
		alGetSourcefv(source_, AL_POSITION, pos);
		return ALVecToVec(Vector3(pos[0], pos[1], pos[2]));
	}

	// 设置声源位置
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::Position(Vector3 const & v)
	{
		Vector3 alv(VecToALVec(v));
		alSourcefv(source_, AL_POSITION, &alv.x());
	}

	// 获取声源速度
	/////////////////////////////////////////////////////////////////////////////////
	Vector3 OALMusicBuffer::Velocity() const
	{
		float vel[3];
		alGetSourcefv(source_, AL_VELOCITY, vel);
		return ALVecToVec(Vector3(vel[0], vel[1], vel[2]));
	}

	// 设置声源速度
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::Velocity(Vector3 const & v)
	{
		Vector3 alv(VecToALVec(v));
		alSourcefv(source_, AL_VELOCITY, &alv.x());
	}

	// 获取声源方向
	/////////////////////////////////////////////////////////////////////////////////
	Vector3 OALMusicBuffer::Direction() const
	{
		float dir[3];
		alGetSourcefv(source_, AL_DIRECTION, dir);
		return ALVecToVec(Vector3(dir[0], dir[1], dir[2]));
	}

	// 设置声源方向
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::Direction(Vector3 const & v)
	{
		Vector3 alv(VecToALVec(v));
		alSourcefv(source_, AL_DIRECTION, &alv.x());
	}
}
