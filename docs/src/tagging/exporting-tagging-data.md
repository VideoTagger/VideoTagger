# Exporting Tagging Data

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

### Object definitions:
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

# Example

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