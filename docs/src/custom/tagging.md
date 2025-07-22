@page tagging Tagging

[TOC]

## Basics

In order to start tagging a video or a group of videos you need to have at least one tag (see [Tag Management](./tag-management.md)) and have a video group currently playing (see [Video Management](../projects/video-management)). Once that's all setup click the plus button on the `timeline` and select the tags you want to use. Next, right-click on the timeline. A menu will appear with the following options:
- Add timestamp
- Add segment
- Add timestamp at marker
- Add segment at marker
- Start segment at marker
- End segment at marker

After selecting one of the first four options, a popup will appear. If you selected one of the options with `"timestamp"`, you'll be able to select the time at which the timestamp will be created. If you selected one of the options with `"segment"`, you'll be able to select where the segment will start and end. Clicking `"OK"` will create the timestamp or segment.

Selecting `"Start segment at marker"` won't have immediately visible effects. However, once you click `"End segment at marker"`, a popup will appear, showing the start and end times set to the positions of the time marker when `"Start segment at marker"` and `"End segment at marker"` were selected, respectively. If you click `"Start segment at marker"` and then `"End segment at marker"` without moving the time marker, a timestamp will be created instead of a segment.

If you try to create a timestamp/segment which would overlap an already existing timestamp/segment a popup will appear asking whether to merge the segments. Selecting `"Yes"` will merge the segment you want to create and the existing segment, while selecting `"No"` will cancel the timestamp/segment creation.

<div class="warning">

Timestamps and segments are saved on a per video group basis.

</div>

## Tag Management

All tag related operations such as adding, modifying and removing a tag can be done via the `Tag Manager` window.

All tags shown there are stored in the project file and can be used on the `Timeline` window to create `segments` or `timestamps`. To learn more about the `Timeline` head over to [Video Timeline](./timeline.md) page.

![Tag Manager](/images/preview4.png)

## Exporting Tagging Data

In order to export tagging data of the active video group (see [Video Management](../projects/video-management) for how to use video groups) select the `Export Segments` option from the `Import/Export` list in the `File` menu.

![Export Segments](/images/export-segments.png)

The tagging data in saved in a JSON file with the following structure:

```json
{
	"version": "int",

	"groups": [
		{
			"name": "string",
			"id": "string",

			"videos": [
				"video-obj" // one or more
			],

			"tags": [
				"string" // tag name, one or more
			],

			"segments": {
				"video-id": [ // 'video-id' is one of the ids from the videos list. Every video from the videos list must be represented
					{
						"tag": "tag-name", // 'tag-name' is one of the tags from the tags list
						"segments": [
							"segment-obj", // zero or more
							"timestamp-obj"  // zero or more
						]
					}
				]
			}
		}
	]
}
```

### Object definitions
```json
"video-obj": {
	"name": "string",
	"id": "string"
}

"segment-obj": {
	"start": "time", // 'time' is a string representing time in the format hh:mm:ss
	"end": "time" // 'time' is a string representing time in the format hh:mm:ss
}

"timestamp-obj":{
	"timestamp": "time" // 'time' is a string representing time in the format hh:mm:ss
}
```

### Example
```json
{
	"version": 1,
	"groups": [
		{
			"name": "Steamboat Willie",
			"id": "13470171703746230198",
			"videos": [
				{
					"name": "Steamboat Willie.mp4",
					"id": "4839065930439805551"
				}
			],
			"tags": [
				"Tag1",
				"Tag2",
				"Tag3",
				"Tag4"
			],
			"segments": {
				"4839065930439805551": [
					{
						"tag": "Tag1",
						"segments": [
							{
								"start": "00:00:04",
								"end": "00:00:08"
							},
							{
								"timestamp": "00:00:16"
							}
						]
					},
					{
						"tag": "Tag2",
						"segments": [
							{
								"start": "00:00:02",
								"end": "00:00:05"
							},
							{
								"start": "00:00:11",
								"end": "00:00:15"
							},
							{
								"timestamp": "00:00:19"
							}
						]
					},
					{
						"tag": "Tag3",
						"segments": [
							{
								"start": "00:00:07",
								"end": "00:00:10"
							},
							{
								"start": "00:00:25",
								"end": "00:00:33"
							}
						]
					},
					{
						"tag": "Tag4",
						"segments": [
							{
								"timestamp": "00:00:19"
							},
							{
								"start": "00:00:36",
								"end": "00:00:46"
							}
						]
					}
				]
			}
		}
	]
}
```
